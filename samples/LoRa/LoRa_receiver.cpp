#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>


SX1262 lora = new Module(SS, DIO0, RST_LoRa, BUSY_LoRa);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("LoRa Receiver Example");
  int state = lora.begin(433.0,125.0,7,5);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("LoRa initialized successfully!");
  } else {
    Serial.print("LoRa initialization failed, code: ");
    Serial.println(state);
    while (true);
  }
}
int totalBytesReceived = 0;
void loop() {
  String received;
  int state = lora.receive(received);

  if(state == RADIOLIB_ERR_NONE){
    Serial.println("Received packet : " +received);
  }
}
