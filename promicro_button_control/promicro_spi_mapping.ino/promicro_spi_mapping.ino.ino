#include "Adafruit_TinyUSB.h"

int pins[] = {8, 9, 11, 12, 7, 13, 6, 14, 15, 5};
const int NUM_BTNS = 10;

// Analog pin (P0.31)
#define JOY_X A2

uint16_t buttonState = 0;

enum {
  BTN_0, BTN_1, BTN_2, BTN_3,
  BTN_4, BTN_5, BTN_6, BTN_7,
  BTN_8, BTN_9
};

void setup() {
  Serial.begin(115200);   // Debug
  Serial1.begin(115200);  // UART to Pi

  for (int i = 0; i < NUM_BTNS; i++) {
    pinMode(pins[i], INPUT_PULLUP);
  }
}

void loop() {
  uint16_t state = 0;

  // Read buttons
  for (int i = 0; i < NUM_BTNS; i++) {
    if (digitalRead(pins[i]) == LOW) {
      state |= (1 << i);
    }
  }

  // Combo: BTN_9 + BTN_8
  if ((state & (1 << BTN_9)) && (state & (1 << BTN_8))) {
    state |= (1 << 10); // virtual button
  }

  // Read analog (0–1023)
  int joyX = analogRead(JOY_X);

  // Compress to 1 byte (0–255)
  uint8_t joyX8 = map(joyX, 0, 1023, 0, 255);

  // Debug
  Serial.print("Buttons: ");
  Serial.print(state);
  Serial.print(" | X: ");
  Serial.println(joyX8);

  // Send: [buttons (2 bytes)] + [analog (1 byte)]
  Serial1.write((uint8_t*)&state, 2);
  Serial1.write(joyX8);

  delay(10);
}