#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

SX1262 lora = new Module(SS, DIO0, RST_LoRa, BUSY_LoRa);

// ================= BLE UUIDs =================
#define SERVICE_UUID            "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID_RX  "12345678-1234-5678-1234-56789abcdef1"
#define CHARACTERISTIC_UUID_TX  "12345678-1234-5678-1234-56789abcdef2"

BLECharacteristic *txChar;
BLECharacteristic *rxChar;
BLEAdvertising *advertising;

// Queue for BLE → LoRa messages
String bleMessage = "";
bool hasNewMessage = false;

// ================= BLE Callback =================
class RXCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *characteristic) {
    std::string value = characteristic->getValue();
    if (value.length() > 0) {
      bleMessage = value.c_str();
      hasNewMessage = true;
      Serial.print("[BLE] Received from phone: ");
      Serial.println(bleMessage);
    }
  }
};

class mypServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer* pServer) {
    Serial.println("Client connected");
  }
  void onDisconnect(BLEServer* pServer) {
    Serial.println("Client disconnected");
    advertising->start();
  }
};


void setup() {
  Serial.begin(115200);
  while (!Serial);

  // ================= LoRa Setup =================
  Serial.println("[LoRa] Initializing...");
  int state = lora.begin(433.0, 125.0, 7, 5); // freq, bw, sf, cr
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("[LoRa] init failed, code: ");
    Serial.println(state);
    while (true);
  }
  lora.setOutputPower(17); // 2-22 dBm for SX1262
  Serial.println("[LoRa] init success");

  // ================= BLE Setup =================
  BLEDevice::init("LoRa-BLE iphone"); // name seen in LightBlue

  BLEServer *pServer = BLEDevice::createServer();

  BLEService *service = pServer->createService(SERVICE_UUID);

  txChar = service->createCharacteristic(
    CHARACTERISTIC_UUID_TX,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  txChar->addDescriptor(new BLE2902());
  rxChar = service->createCharacteristic(
    CHARACTERISTIC_UUID_RX,
    BLECharacteristic::PROPERTY_WRITE
  );

  rxChar->setCallbacks(new RXCallback());
  pServer->setCallbacks(new mypServerCallbacks());

  service->start();

  advertising = pServer->getAdvertising();
  advertising->start();
  Serial.println("[BLE] Advertising started");
}

void loop() {
  // ================= LoRa TX from BLE =================
  if (hasNewMessage) {
    hasNewMessage = false;
    int state = lora.transmit(bleMessage.c_str());
    delay(100);

    if (state == RADIOLIB_ERR_NONE) {
      Serial.print("[LoRa] Sent: ");
      Serial.println(bleMessage);
    }

  }

  // ================= LoRa RX to BLE =================
  String msg;
  int state = lora.receive(msg);
  delay(100);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.print("[LoRa] Received: ");
    Serial.println(msg);

    txChar->setValue(msg.c_str());
    txChar->notify();   // forward to phone via BLE
  }
}
