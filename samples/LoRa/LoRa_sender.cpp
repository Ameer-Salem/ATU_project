#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>

SX1262 lora = new Module(SS, DIO0, RST_LoRa, BUSY_LoRa);
String myString = "";
int state;

void setup()
{

  Serial.begin(115200); // start serial communication
  while (!Serial)
    ; // wait for serial port to connect (for some boards)

  state = lora.begin(433.0, 125.0, 7, 5);

  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println("LoRa initialized successfully!");
  }
  else
  {
    Serial.print("LoRa initialization failed, code : ");
    Serial.println(state);
    while (true)
      ;
  }

  Serial.print("Type something and press Enter : ");
}
int delayTime = 1000;
void loop()
{
  if (Serial.available())
  {

    Serial.println("Got it! Waiting a second...");
    delay(delayTime); // wait a few seconds

    myString = Serial.readStringUntil('\n');
    myString.trim();
    if (myString != "")
    {
      do
      {
        state = lora.transmit(myString);
        if (state == RADIOLIB_ERR_NONE)
        {
          Serial.println("Sent packet! : " + myString);
          myString = "";
        }
        else
        {
          Serial.print("Transmit failed, code : ");
          Serial.println(state);
          Serial.printf("Retrying afer %d second/s...",delayTime/1000);
          delay(delayTime);
        }
      } while (state != RADIOLIB_ERR_NONE);
      Serial.print("\nType something and press Enter again : ");
    } // prompt again
  }
}
