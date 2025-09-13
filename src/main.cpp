#include <Arduino.h>
#include "Packet.h"
#include "LoRaHandler.h"

#define MY_ID 0x01
#define DESTINATION_ID 0x02

LoRaHandler lora(SS, DIO0, RST_LoRa, BUSY_LoRa);

uint8_t sequence = 0;
bool waitingAck = false;
unsigned long previousMillis = 0;

void setup()
{
  Serial.begin(115200);

  if (!lora.begin())
  {
    /* code */
    Serial.println("LoRa initialization failed!");
    while (true)
      ;
  }

  Serial.println("LoRa initialized successfully!");
}

void loop()
{
  Packet rx;
  if (millis() - previousMillis >= 3250)
  {
    if (!waitingAck)
    {
      lora.sendText("Hello from node " + String(MY_ID), MY_ID, DESTINATION_ID, sequence++);
      waitingAck = true;
    }
    previousMillis = millis();
  }

  if (lora.receive(rx))
  {
    /* code */
    Serial.println("Received packet!");
    Serial.println("Type : " + String(rx.type == TEXT_TYPE ? "TEXT" : "ACK"));
    if (rx.type == TEXT_TYPE)
    {
      /* code */
      lora.sendAck(rx.sequence, rx.source);
    }
    else if (rx.type == ACK_TYPE)
    {
      /* code */
      waitingAck = false;
    }
    
  }
}
