#include <IRremote.h>

const int irLedPin = 3;
const int speakerPin = 9;  // Use a PWM-capable pin
const int buttonPin = 2;
uint32_t message = 0xB1B1B1B1;


IRsend irsend;

void setup() {
  pinMode(buttonPin, INPUT);
  IrSender.begin(irLedPin);  // Start IR transmitter on pin 8
}

void playPew() {
  for (int freq = 1200; freq >= 200; freq -= 20) {
    tone(speakerPin, freq);
    delay(10);
  }
  noTone(speakerPin);
}

void loop() {
  if (digitalRead(buttonPin) == HIGH) {
    // Send a specific code using NEC protocol
    IrSender.sendNEC(message, 32);  // 32-bit example code
    playPew();
    delay(300);  // Debounce delay
  }
}