#include <Arduino.h>

void setup() {

  pinMode(LED, OUTPUT);

  Serial.begin(115200);
  Serial.println("Serial Monitor initialized.");

}

void loop() {
  
  digitalWrite(LED, HIGH);
  Serial.println("LED ON");
  delay(1000);

  digitalWrite(LED, LOW);
  Serial.println("LED OFF");
  delay(2000);
  
}