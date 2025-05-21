#include <IRremote.h>

const int irReceiverPin = 2;
const int ledPinRed = 13;
const int ledPinYellow = 12;

unsigned long startTime;
unsigned long roundStartTime;

bool shotBeforeLight = false;
bool targetShot = true;
bool roundStart = true;
bool needSetup = false;

int randomInt;



void setup() {
  pinMode(ledPinRed, OUTPUT);
  pinMode(ledPinYellow, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(2), irISR, RISING);
  Serial.begin(9600);
  IrReceiver.begin(irReceiverPin, ENABLE_LED_FEEDBACK);  // Enable IR receiver on pin 2
}

uint32_t fixBits(uint32_t val) {
  uint32_t result = 0;
  for (int i = 0; i < 32; i++) {
    result <<= 1;
    result |= (val & 1);
    val >>= 1;
  }
  return result;
}

void irISR() {
  if(digitalRead(12) == HIGH){
    shotBeforeLight = false;
  }else{
    shotBeforeLight = true;
  }
}


void loop() {
  if(roundStart){
    roundStartTime = millis();
    randomInt = random(1,5);
    roundStart = false;
    needSetup = true;
  }
  
  if ((millis() - roundStartTime) > (randomInt * 1000) && needSetup){
    digitalWrite(ledPinYellow, HIGH);
    startTime = millis();
    needSetup = false;
  }
  
  if (IrReceiver.decode()) {
    digitalWrite(ledPinRed, HIGH);
    unsigned long totalTime = millis() - startTime;
    // Print received code
    Serial.print("Reversed raw: 0x");
    Serial.println(fixBits(IrReceiver.decodedIRData.decodedRawData), HEX);
    if(!shotBeforeLight){
      Serial.print("Reaction time: ");
      Serial.println(totalTime);
      digitalWrite(ledPinYellow, LOW);
      roundStart = true;
    }else{
      Serial.println("Verkakt");
      shotBeforeLight = false;
    }
    IrReceiver.resume();  // Prepare for the next signal
    delay(100);  // Optional: avoid flooding
  }
}