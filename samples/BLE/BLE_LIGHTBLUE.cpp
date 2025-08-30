#include <Arduino.h>
#include <BLEDevice.h>
#include <BLE2902.h>

#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcd1234-ab12-cd34-ef56-1234567890ab"

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Client connected");
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Client disconnected");
  }
};


class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();
    if (rxValue.length() > 0) {
      Serial.print("Received Value: ");
      
      Serial.print(rxValue.c_str());
      Serial.println();
      // You can also process the received data here
    }
  }
};



BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();




void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Server...");

  BLEDevice::init("Heltec_WiFi_LoRa");  // Device name
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_WRITE
                    );
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->setValue("Hello from Heltec!");

  pService->start();

  pAdvertising->start();
  Serial.println("Waiting for client to connect...");
}

void loop() {
  int lastNotifyTime = 0 ;//2001;
  if (!deviceConnected) {
    // You can update characteristic value or notify client here
    pAdvertising->start();
    Serial.println("Waiting for client to connect...");
    delay(10000);
  }
  else
  {
    
    long now = millis(); // 4002
    if (now - lastNotifyTime > 2000) {  // notify every 2 seconds
      int count = 0;
      char buffer[20];
      sprintf(buffer, "Count: %d", count++);
      pCharacteristic->setValue(buffer);
      pCharacteristic->notify();
      Serial.print("Notified: ");
      Serial.println(buffer);
      lastNotifyTime = now;
    }
  
  }
  
  delay(1000);
}
