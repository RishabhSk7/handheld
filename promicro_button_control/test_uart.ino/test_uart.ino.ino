#include "Adafruit_TinyUSB.h"

void setup() {
  Serial1.begin(115200);
}

void loop() {
  Serial1.println("HELLO...");
  delay(1000);
  Serial1.println("Kaafi din huye");
  delay(1000);
  Serial1.println("Bike trip nahi krri T_Ts");
  delay(3000);
}