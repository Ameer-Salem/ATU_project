#include "ble.h"

String bleMessage = "";
bool hasNewMessage = false;

BLEAdvertising *advertising;
BLEServer *pServer;
BLEService *pService;
BLECharacteristic *txChar;
BLECharacteristic *rxChar;


void RXCallback::onWrite(BLECharacteristic *characteristic)
{
    uint8_t* pValue = characteristic->getData();
    int len = characteristic->getLength();
        if (len > 0)
        {
            Packet packet;
            
            packet.type = pValue[0];
            packet.source = pValue[1];
            packet.destination = pValue[2];
            packet.sequence = pValue[3];
            packet.length = pValue[4];
            for (int i = 0; i < packet.length; i++)
            {
                packet.payload[i] = pValue[5 + i];
            }
            bleMessage = getPayload(packet);
            outgoingQueue.push(packet);
            sLog(BLE_TAG, "Received from phone: " + bleMessage);
        }
}

void MyServerCallbacks::onConnect(BLEServer *pServer)
{
    sLog(BLE_TAG, "Client connected");
};

void MyServerCallbacks::onDisconnect(BLEServer *pServer)
{
    sLog(BLE_TAG, "Client disconnected");
    advertising->start();
};

void bleSetup()
{
    BLEDevice::init(NODE_NAME.c_str());
    pServer = BLEDevice::createServer();
    pService = pServer->createService(SERVICE_UUID);
    txChar = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
    txChar->addDescriptor(new BLE2902());
    rxChar = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
    rxChar->setCallbacks(new RXCallback());
    pServer->setCallbacks(new MyServerCallbacks());
    pService->start();
    advertising = pServer->getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->start();
    sLog(BLE_TAG, "Advertising started");
}
