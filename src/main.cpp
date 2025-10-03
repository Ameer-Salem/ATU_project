// ===================== Includes =====================
#include <RadioLib.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <queue>

std::queue<String> outgoingQueue;
std::queue<String> ingoingQueue;
// ===================== BLE Definitions =====================
#define SERVICE_UUID "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID_RX "12345678-1234-5678-1234-56789abcdef1"
#define CHARACTERISTIC_UUID_TX "12345678-1234-5678-1234-56789abcdef2"
#define BLE_TAG "[BLE]    "

// ===================== LoRa Definitions =====================
#define LORA_TAG "[SX1262] "
#define NODE_ID 1
String NODE_NAME = "Node " + String(NODE_ID);

// ===================== BLE Globals =====================
BLECharacteristic *txChar;
BLECharacteristic *rxChar;
BLEAdvertising *advertising;
String bleMessage = "";
bool hasNewMessage = false;

// ===================== LoRa Globals =====================
Module loraModule(SS, DIO0, RST_LoRa, BUSY_LoRa);
SX1262 radio(&loraModule);
int transmissionState = RADIOLIB_ERR_NONE;
bool transmitFlag = false;
volatile bool operationDone = false;

// ===================== Packet Definitions =====================
struct Packet
{
    uint8_t type;
    uint8_t source;
    uint8_t destination;
    uint8_t sequence;
    uint8_t length;
    uint8_t payload[200];
};
Packet tx, rx;
uint8_t buffer[206];

// ===================== Protocol Constants =====================
#define MAX_RETRIES 3
#define ACK_TIMEOUT 5000 // ms
#define SOURCE_ID NODE_ID
#define DESTINATION_ID 2
uint8_t TEXT_TYPE = 0x01;
uint8_t ACK_TYPE = 0x02;

// ===================== Simulation Settings =====================
#define SIMULATE_PACKET_LOSS 1
#define LOSS_PROBABILITY 30
#define LOSS_ACK_PROBABILITY 10
#define DROP_EVERY_NTH 0
#define DROP_ACK_EVERY_NTH 0
#define STATS_INTERVAL 15000UL

unsigned long lastStatsTime = 0;
unsigned long totalSent = 0;
unsigned long totalAcks = 0;
unsigned long totalRetries = 0;
unsigned long totalFailed = 0;

// ===================== Sequence & ACK Control =====================
static uint8_t sequenceCounter = 0;
static uint8_t lastSequence = 0;
unsigned long ackStartTime = 0;
int retryCount = 0;
bool waitForAck = false;

// ===================== Utility Functions =====================
void sLog(const char *tag, const String &content)
{
    unsigned long now = millis();
    char timeStr[20];
    snprintf(timeStr, sizeof(timeStr), "%02lu:%02lu:%02lu", now / 3600000, (now / 60000) % 60, (now / 1000) % 60);
    Serial.print(timeStr);
    Serial.print(" ");
    Serial.print(tag);
    Serial.print(" ");
    Serial.println(content);
}

// ===================== BLE Callbacks =====================
class RXCallback : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *characteristic)
    {
        std::string value = characteristic->getValue();
        if (value.length() > 0)
        {
            bleMessage = value.c_str();
            outgoingQueue.push(bleMessage);
            sLog(BLE_TAG, "Received from phone: " + bleMessage);
        }
    }
};
class mypServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        sLog(BLE_TAG, "Client connected");
    }
    void onDisconnect(BLEServer *pServer)
    {
        sLog(BLE_TAG, "Client disconnected");
        advertising->start();
    }
};

// ===================== Packet Handling =====================
void setPayload(Packet &tx, const String &myString)
{
    int len = min((int)myString.length(), (int)sizeof(tx.payload));
    for (int i = 0; i < len; i++)
    {
        tx.payload[i] = myString[i];
    }
}
void setPacket(Packet &tx, const String &myString, uint8_t type, uint8_t source, uint8_t destination, uint8_t sequence)
{
    tx.type = type;
    tx.source = source;
    tx.destination = destination;
    tx.sequence = sequence;
    tx.length = myString.length();
    setPayload(tx, myString);
}
void toRaw(Packet &tx)
{
    buffer[0] = tx.type;
    buffer[1] = tx.source;
    buffer[2] = tx.destination;
    buffer[3] = tx.sequence;
    buffer[4] = tx.length;
    for (int i = 0; i < tx.length; i++)
    {
        buffer[5 + i] = tx.payload[i];
    }
}
void fromRaw(uint8_t buffer[], int len)
{
    rx.type = buffer[0];
    rx.source = buffer[1];
    rx.destination = buffer[2];
    rx.sequence = buffer[3];
    rx.length = buffer[4];
    int safeLen = min(rx.length, (uint8_t)sizeof(rx.payload));
    for (int i = 0; i < safeLen; i++)
    {
        rx.payload[i] = buffer[5 + i];
    }
    rx.length = safeLen;
}
String getPayload(uint8_t payload[], int length)
{
    return String((char *)payload, length);
}

// ===================== LoRa Control =====================
void setFlag(void)
{
    operationDone = true;
}
void sendPacket(uint8_t type, uint8_t source, uint8_t destination, uint8_t sequence, String &payload)
{
    setPayload(tx, payload);
    setPacket(tx, payload, type, source, destination, sequence);
    toRaw(tx);

    // Simulation: drop outgoing frames
    if (SIMULATE_PACKET_LOSS)
    {
        if (type == TEXT_TYPE && DROP_EVERY_NTH > 0 && (sequence % DROP_EVERY_NTH) == 0)
        {
            sLog(LORA_TAG, "SIMULATED DROP of DATA packet seq " + String(sequence));
            transmissionState = RADIOLIB_ERR_NONE;
            transmitFlag = true;
            totalSent++;
            return;
        }
        if (type == ACK_TYPE && DROP_ACK_EVERY_NTH > 0 && (sequence % DROP_ACK_EVERY_NTH) == 0)
        {
            sLog(LORA_TAG, "SIMULATED DROP of ACK seq " + String(sequence));
            transmissionState = RADIOLIB_ERR_NONE;
            transmitFlag = true;
            return;
        }
        int r = random(100);
        if (type == TEXT_TYPE && r < LOSS_PROBABILITY)
        {
            sLog(LORA_TAG, "SIMULATED RANDOM DROP of DATA packet seq " + String(sequence) + " (r=" + String(r) + ")");
            transmissionState = RADIOLIB_ERR_NONE;
            transmitFlag = true;
            totalSent++;
            return;
        }
        if (type == ACK_TYPE && r < LOSS_ACK_PROBABILITY)
        {
            sLog(LORA_TAG, "SIMULATED RANDOM DROP of ACK seq " + String(sequence) + " (r=" + String(r) + ")");
            transmissionState = RADIOLIB_ERR_NONE;
            transmitFlag = true;
            return;
        }
    }

    // Real transmit
    transmissionState = radio.startTransmit(buffer, tx.length + 5);
    if (transmissionState == RADIOLIB_ERR_NONE)
    {
        transmitFlag = true;
        totalSent++;
        sLog(LORA_TAG, "transmission started! type : " + String(type == (uint8_t)TEXT_TYPE ? "TEXT" : "ACK") + " seq " + String(sequence));
    }
    else
    {
        sLog(LORA_TAG, "failed, code " + String(transmissionState));
    }
}
void sendACK()
{
    String ackStr = "ACK:" + String(rx.sequence);
    sendPacket(ACK_TYPE, rx.destination, rx.source, rx.sequence, ackStr);
}
void startListening()
{
    sLog(LORA_TAG, "Starting to listen ... ");
    int state = radio.startReceive();
    if (state != RADIOLIB_ERR_NONE)
    {
        sLog(LORA_TAG, "failed, code " + String(state));
    }
}
void retrySend()
{
    if (retryCount < MAX_RETRIES)
    {
        String str = getPayload(tx.payload, tx.length);
        retryCount++;
        totalRetries++;
        sLog(LORA_TAG, "Retrying... attempt " + String(retryCount));
        sendPacket(TEXT_TYPE, SOURCE_ID, DESTINATION_ID, sequenceCounter, str);
        ackStartTime = millis();
    }
    else
    {
        sLog(LORA_TAG, "Failed to send after " + String(MAX_RETRIES) + " attempts");
        totalFailed++;
        waitForAck = false;
    }
}
void sendText(String &str)
{
    sequenceCounter++;
    retryCount = 0;
    sendPacket(TEXT_TYPE, SOURCE_ID, DESTINATION_ID, sequenceCounter, str);
    ackStartTime = millis();
    waitForAck = true;
    sLog(LORA_TAG, "Waiting for ACK...");
}

// ===================== Arduino Setup =====================
void setup()
{
    randomSeed(analogRead(13));
    Serial.begin(115200);

    // BLE setup
    BLEDevice::init(NODE_NAME.c_str());
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *service = pServer->createService(SERVICE_UUID);

    txChar = service->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
    txChar->addDescriptor(new BLE2902());
    rxChar = service->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
    rxChar->setCallbacks(new RXCallback());
    pServer->setCallbacks(new mypServerCallbacks());
    service->start();
    advertising = pServer->getAdvertising();
    advertising->start();
    sLog(BLE_TAG, "Advertising started");

    // LoRa setup
    sLog(LORA_TAG, "Initializing ... ");
    int state = radio.begin();
    if (state != RADIOLIB_ERR_NONE)
    {
        sLog(LORA_TAG, "failed, code " + String(state));
        while (true)
        {
            delay(10);
        }
    }
    radio.setDio1Action(setFlag);
    startListening();
}

// ===================== Arduino Loop =====================
void loop()
{
    // Handle LoRa events
    if (operationDone)
    {
        sLog(LORA_TAG, "Previous operation finished");
        operationDone = false;
        if (transmitFlag)
        {
            sLog(LORA_TAG, "Previous operation was transmission");
            transmitFlag = false;
            if (transmissionState != RADIOLIB_ERR_NONE)
            {
                sLog(LORA_TAG, "failed, code " + String(transmissionState));
            }
            startListening();
        }
        else
        {
            sLog(LORA_TAG, "Previous operation was reception");
            memset(buffer, 0, sizeof(buffer));
            memset(&rx, 0, sizeof(rx));
            int len = radio.getPacketLength();
            int state = radio.readData(buffer, len);
            sLog(LORA_TAG, "RSSI: " + String(radio.getRSSI()) + " dBm");
            sLog(LORA_TAG, "SNR: " + String(radio.getSNR()) + " dB");
            if (state != RADIOLIB_ERR_NONE)
            {
                sLog(LORA_TAG, "failed, code " + String(state));
                return;
            }
            if (len < 5 || buffer[4] > sizeof(rx.payload))
            {
                sLog(LORA_TAG, "Invalid packet received");
                return;
            }
            fromRaw(buffer, len);
            String str = getPayload(rx.payload, rx.length);
            if (rx.type == ACK_TYPE)
            {
                sLog(LORA_TAG, "Received ACK! Sequence " + String(rx.sequence));
                totalAcks++;
                waitForAck = false;
            }
            else
            {
                if (rx.sequence != lastSequence)
                {
                    lastSequence = rx.sequence;
                    ingoingQueue.push(str);
                    sLog(LORA_TAG, "Buffered new TEXT: " + str);
                }
                else
                {
                    sLog(LORA_TAG, "Duplicate TEXT received, sequence " + String(rx.sequence));
                }

                sendACK();
            }
        }
    }

    // Handle ACK timeout
    if (waitForAck && (millis() - ackStartTime > ACK_TIMEOUT))
    {
        retrySend();
    }

    // Handle BLE message
    if (!outgoingQueue.empty() && !waitForAck)
    {
        String nextMessage = outgoingQueue.front();
        outgoingQueue.pop();
        sendText(nextMessage);
    }
    if (!ingoingQueue.empty())
    {
        String str = ingoingQueue.front();
        ingoingQueue.pop();
        txChar->setValue(str.c_str());
        txChar->notify();
        sLog(BLE_TAG, "Forwarded Buffered TEXT: " + str);
    }
    // Print stats
    if (millis() - lastStatsTime > STATS_INTERVAL)
    {
        lastStatsTime = millis();
        sLog(LORA_TAG, "STATS: sent=" + String(totalSent) + " acks=" + String(totalAcks) + " retries=" + String(totalRetries) + " failed=" + String(totalFailed));
    }
}
