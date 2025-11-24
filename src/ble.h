#pragma once
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "packet.h"
#include "lora.h"

extern BLECharacteristic *notifyChar;
extern BLECharacteristic *writeChar;
extern BLECharacteristic *readChar;
extern BLEAdvertising *advertising;
extern BLEServer *pServer;
extern BLEService *pService;
extern  String bleMessage;
extern  bool hasNewMessage;

void bleSetup();
void notifyBLE(std::vector<uint8_t> buffer);
class RXCallback : public BLECharacteristicCallbacks
{
public:
    void onWrite(BLECharacteristic *characteristic) override;
};

class MyServerCallbacks : public BLEServerCallbacks
{
public:
    void onConnect(BLEServer *pServer) override;
    void onDisconnect(BLEServer *pServer) override;
};