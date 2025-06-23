#include <IRremote.h>

const int irLedPin = 2;
const int speakerPin = 6;  // Use a PWM-capable pin
const int buttonPin = 4;
const int laserPin = 3;
const int motorPin = 5;

const int vibrationStrength = 150;
const int feedbackDuration = 100; //in ms

uint32_t message = 0xB1B1B1B1;


IRsend irsend;

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(laserPin, OUTPUT);
  pinMode(motorPin, OUTPUT);
  IrSender.begin(irLedPin);  // Start IR transmitter on pin 8
}

void playPew() {
  for (int freq = 1200; freq >= 200; freq -= 20) {
    tone(speakerPin, freq);
    delay(10);
  }
  noTone(speakerPin);
}

void shootLaser(){
  digitalWrite(laserPin, HIGH);
  delay(100);
  digitalWrite(laserPin, LOW);
}

void vibrateMotor(){
  analogWrite(motorPin, vibrationStrength);
  delay(100);
  analogWrite(motorPin, 0);
}

void loop() {
  if (digitalRead(buttonPin) == HIGH) {
    IrSender.sendNEC(message, 32);  // 32-bit example code
    shootLaser();
    // Send a specific code using NEC protocol
    vibrateMotor();
    
    delay(300);  // Debounce delay
  }
}