#include "ble.h"

String bleMessage = "";
bool hasNewMessage = false;

BLEAdvertising *advertising;
BLEServer *pServer;
BLEService *pService;
BLECharacteristic *notifyChar;
BLECharacteristic *writeChar;
BLECharacteristic *readChar;

uint64_t intNODE_ID;
String NODE_ID;

void RXCallback::onWrite(BLECharacteristic *characteristic)
{
    uint8_t *pValue = characteristic->getData();
    int len = characteristic->getLength();
    if (len > 0)
    {
        Packet packet = fromRaw(pValue, len);
        if (packet.type != ACK_TYPE)
        {
            bleMessage = getPayload(packet);

            outgoingQueue.push(packet);
            sLog(BLE_TAG,
                 " \n===============\n "
                 "Received from BLE: " +
                     String(packet.type) + " : " +
                     ((char *)packet.source) + " : " +
                     ((char *)packet.destination) + " : " +
                     ((char *)packet.uuid) + " : " +
                     String(packet.segmentIndex) + " : " +
                     String(packet.totalSegments) + " : " +
                     String(packet.length) + " : " +
                     bleMessage +
                     " \n===============\n ");
        }
        else
        {
            sLog(BLE_TAG, "Transmiting ACK...");
            ackQueue.push(packet);
        }
    }
}

void MyServerCallbacks::onConnect(BLEServer *pServer)
{
    sLog(BLE_TAG, "Client connected");
    memset(&buffer, 0, sizeof(buffer));
    bleMessage = "";
    hasNewMessage = false;
    operationDone = false;
    transmitFlag = false;
};

void MyServerCallbacks::onDisconnect(BLEServer *pServer)
{
    sLog(BLE_TAG, "Client disconnected");
    advertising->start();
};
String getBoardId()
{
    uint64_t chipid = ESP.getEfuseMac();
    intNODE_ID = chipid; // 48-bit chip ID
    char id[13];                         // 12 hex chars + null terminator
    sprintf(id, "%012llX", chipid);
    return String(id);
}
void bleSetup()
{
    BLEDevice::init(NODE_NAME.c_str());
    pServer = BLEDevice::createServer();
    pService = pServer->createService(SERVICE_UUID);

    readChar = pService->createCharacteristic(CHARACTERISTIC_UUID_read, BLECharacteristic::PROPERTY_READ);
    NODE_ID = getBoardId();
    readChar->setValue(NODE_ID.c_str());

    notifyChar = pService->createCharacteristic(CHARACTERISTIC_UUID_notify, BLECharacteristic::PROPERTY_NOTIFY);
    notifyChar->addDescriptor(new BLE2902());

    writeChar = pService->createCharacteristic(CHARACTERISTIC_UUID_write, BLECharacteristic::PROPERTY_WRITE);
    writeChar->setCallbacks(new RXCallback());

    pServer->setCallbacks(new MyServerCallbacks());
    pService->start();
    advertising = pServer->getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->start();
    sLog(BLE_TAG, "Advertising started");
}
