#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(2000); // give USB time to connect

  Serial.println("BOOT OK");

  pinMode(31, INPUT_PULLUP);
  pinMode(29, INPUT_PULLUP);
  pinMode(17, INPUT_PULLUP);
  pinMode(20, INPUT_PULLUP);
}

void loop() {
  Serial.print(digitalRead(31));
  Serial.print(" ");
  Serial.print(digitalRead(29));
  Serial.print(" ");
  Serial.print(digitalRead(17));
  Serial.print(" ");
  Serial.println(digitalRead(20));

  delay(500);
}