#pragma once
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "packet.h"
#include "lora.h"

extern BLECharacteristic *txChar;
extern BLECharacteristic *rxChar;
extern BLEAdvertising *advertising;
extern BLEServer *pServer;
extern BLEService *pService;
extern  String bleMessage;
extern  bool hasNewMessage;

void bleSetup();

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