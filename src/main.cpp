#include <Arduino.h> 
#include <SPI.h>
#include <RadioLib.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// ================== LoRa PIN MAP (Heltec V3, SX1262) ==================
#define LORA_SS    8     // NSS
#define LORA_DIO1  14    // DIO1
#define LORA_RST   12    // RESET
#define LORA_BUSY  13    // BUSY

SX1262 lora = new Module(LORA_SS, LORA_DIO1, LORA_RST, LORA_BUSY);

// ================== BLE UUIDs ==================
#define SERVICE_UUID            "22345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID_RX  "22345678-1234-5678-1234-56789abcdef1"
#define CHARACTERISTIC_UUID_TX  "22345678-1234-5678-1234-56789abcdef2"

// Give each device a distinct BLE name for clarity
#ifndef BLE_NAME
#define BLE_NAME "LoRa-BLE Node ipad"
#endif

BLECharacteristic* txChar;
BLECharacteristic* rxChar;
BLEAdvertising* advertising;

// ================== Reliability Layer Config ==================
static const unsigned long ACK_TIMEOUT_MS = 600;   // wait this long for ACK
static const int MAX_RETRIES = 3;                  // retries before giving up
static const size_t LORA_SAFE_PAYLOAD = 200;       // keep under ~200 bytes

// ================== State (TX with ACK) ==================
volatile bool hasNewBLEMessage = false;  // set in RX callback
String pendingBLEPayload = "";           // last payload from phone

// Outgoing message tracking
uint32_t msgCounter = 0;     // increases per message we send
int lastTxState = 0;         // last transmit() return code

bool awaitingAck = false;
uint32_t lastMsgId = 0;
String lastFramedMessage = "";     // "<ID>|<payload>"
unsigned long lastSendTimeMs = 0;
int retryCount = 0;

// ================ Helpers: Framing =================
// frame: "<ID>|<payload>"
static String frameMessage(uint32_t id, const String& payload) {
  return String(id) + "|" + payload;
}

static bool isAck(const String& s, uint32_t& ackIdOut) {
  if (!s.startsWith("ACK|")) return false;
  ackIdOut = s.substring(4).toInt();
  return true;
}

static bool parseFramed(const String& s, uint32_t& idOut, String& payloadOut) {
  int sep = s.indexOf('|');
  if (sep <= 0) return false;
  idOut = s.substring(0, sep).toInt();
  payloadOut = s.substring(sep + 1);
  return true;
}

// ================ BLE Callbacks =================
class RXCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic) override {
    std::string v = characteristic->getValue();
    if (!v.empty()) {
      pendingBLEPayload = String(v.c_str());

      // enforce a max payload for LoRa
      if (pendingBLEPayload.length() > LORA_SAFE_PAYLOAD) {
        pendingBLEPayload = pendingBLEPayload.substring(0, LORA_SAFE_PAYLOAD);
      }

      hasNewBLEMessage = true;
      Serial.print("[BLE] RX from phone: ");
      Serial.println(pendingBLEPayload);
    }
  }
};

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    Serial.println("[BLE] Client connected");
  }
  void onDisconnect(BLEServer* pServer) override {
    Serial.println("[BLE] Client disconnected");
    advertising->start();
  }
};

// ================ LoRa Send with ACK =================
void startSendWithAck(const String& payload) {
  if (awaitingAck) {
    Serial.println("[WARN] Still awaiting ACK; dropping new send request to keep logic simple.");
    return;
  }

  msgCounter++;
  lastMsgId = msgCounter;
  lastFramedMessage = frameMessage(lastMsgId, payload);

  lastTxState = lora.transmit(lastFramedMessage.c_str());
  Serial.printf("[LoRa] TX(id=%lu) state=%d\n", (unsigned long)lastMsgId, lastTxState);

  if (lastTxState == RADIOLIB_ERR_NONE) {
    awaitingAck = true;
    retryCount = 0;
    lastSendTimeMs = millis();
  } else {
    Serial.println("[ERROR] LoRa transmit failed (no ACK phase entered).");
  }
}

void resendLast() {
  lastTxState = lora.transmit(lastFramedMessage.c_str());
  Serial.printf("[LoRa] RETRY TX(id=%lu) state=%d (attempt %d)\n",
                (unsigned long)lastMsgId, lastTxState, retryCount);
  lastSendTimeMs = millis();
}

// ================ Process LoRa Receive =================
void processLoRaReceive() {
  String rx;
  int st = lora.receive(rx);
  if (st == RADIOLIB_ERR_NONE) {
    // Got a packet
    Serial.print("[LoRa] RX raw: ");
    Serial.println(rx);

    // ACK packet?
    uint32_t ackId = 0;
    if (isAck(rx, ackId)) {
      if (awaitingAck && ackId == lastMsgId) {
        awaitingAck = false;
        Serial.printf("[ACK] Received for id=%lu ✅\n", (unsigned long)ackId);
      } else {
        Serial.printf("[ACK] Received for id=%lu (no match/ignored)\n", (unsigned long)ackId);
      }
      return;
    }

    // Normal data -> parse framed "<ID>|<payload>"
    uint32_t recvId = 0;
    String payload;
    if (parseFramed(rx, recvId, payload)) {
      // Send ACK back immediately
      String ack = "ACK|" + String(recvId);
      lora.transmit(ack.c_str());
      Serial.printf("[ACK] Sent for id=%lu\n", (unsigned long)recvId);

      // Forward payload to phone via BLE notify
      if (txChar) {
        txChar->setValue(payload.c_str());
        txChar->notify();
      }
      Serial.print("[BLE] TX to phone: ");
      Serial.println(payload);
    } else {
      // If packet isn’t framed, treat as plain payload (legacy)
      // Forward to phone and do not ACK (no id)
      if (txChar) {
        txChar->setValue(rx.c_str());
        txChar->notify();
      }
      Serial.println("[INFO] Unframed payload forwarded (no ACK sent).");
    }
  }
}

// ================ Check ACK Timeout / Retries =================
void checkAckTimeouts() {
  if (!awaitingAck) return;

  if (millis() - lastSendTimeMs >= ACK_TIMEOUT_MS) {
    if (retryCount < MAX_RETRIES) {
      retryCount++;
      Serial.printf("[Retry] Timeout; retry %d/%d\n", retryCount, MAX_RETRIES);
      resendLast();
    } else {
      Serial.println("[ERROR] Message failed after retries ❌");
      awaitingAck = false;
    }
  }
}

// ================== Setup ==================
void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  // ---- LoRa
  Serial.println("[LoRa] Initializing...");
  // freq=433.0 MHz, bw=125 kHz, sf=7, cr=5(4/5). Adjust as needed for your region/range.
  int st = lora.begin(433.0, 125.0, 7, 5);
  if (st != RADIOLIB_ERR_NONE) {
    Serial.print("[LoRa] init failed, code: ");
    Serial.println(st);
    while (true) { delay(1000); }
  }
  lora.setOutputPower(17); // SX1262: 2..22 dBm
  Serial.println("[LoRa] init success");

  // ---- BLE
  BLEDevice::init(BLE_NAME);
  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService* service = server->createService(SERVICE_UUID);

  txChar = service->createCharacteristic(
    CHARACTERISTIC_UUID_TX,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  txChar->addDescriptor(new BLE2902()); // enables notifications on many clients

  rxChar = service->createCharacteristic(
    CHARACTERISTIC_UUID_RX,
    BLECharacteristic::PROPERTY_WRITE
  );
  rxChar->setCallbacks(new RXCallback());

  service->start();

  advertising = server->getAdvertising();
  advertising->start();
  Serial.println("[BLE] Advertising started");
}

// ================== Loop ==================
void loop() {
  // 1) If phone wrote something to BLE RX, send it over LoRa with ACK
  if (hasNewBLEMessage) {
    hasNewBLEMessage = false;
    // Kick off reliable send
    startSendWithAck(pendingBLEPayload);
  }

  // 2) Always process incoming LoRa packets (ACKs or data)
  processLoRaReceive();

  // 3) Handle resend/timeout if awaiting ACK
  checkAckTimeouts();

  // Small pacing
  delay(10);
}
