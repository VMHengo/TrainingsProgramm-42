#include <IRremote.h>

const int irReceiverPin = 2;
const int ledPinIndicator = 12;

unsigned long reactionTimer;
unsigned long newGameStartTimer;
int randomInt;

bool earlyShot;
bool newRound = true;
bool randomTimePassed;

// -----------------------------------------------------------------

void setup() {
  pinMode(ledPinIndicator, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(2), irISR, RISING);
  Serial.begin(9600);
  IrReceiver.begin(irReceiverPin, DISABLE_LED_FEEDBACK);  // Enable IR receiver on pin 2
}

void loop() {
  // Benötigter Setup bei Rundenstart, um die LED nach [randomInt] Sekunden aufleuchten zu lassen
  if(newRound){
    newGameStartTimer = millis();
    randomInt = random(1,5);
    newRound = false;
    randomTimePassed = false;
  }

  // Sobald mehr Zeit vergangen ist als [randomInt] Sekunden, leuchter die Indikator LED auf
  if ((millis() - newGameStartTimer) > (randomInt * 1000) && !randomTimePassed){
    digitalWrite(ledPinIndicator, HIGH);
    reactionTimer = millis();
    randomTimePassed = true;
  }
  
  // Wird ausgeführt, wenn auf den Receiver geschossen wurde
  if (IrReceiver.decode()) {
    // Print received code
    Serial.print("Reversed raw: 0x");
    Serial.println(fixBits(IrReceiver.decodedIRData.decodedRawData), HEX);
    if(!earlyShot){
      // Berechne die Reaktionszeit
      unsigned long totalTime = millis() - reactionTimer;
      Serial.print("Reaction time: ");
      Serial.println(totalTime);
      digitalWrite(ledPinIndicator, LOW);
      newRound = true;
    }else{
      Serial.println("Verkakt");
      earlyShot = false;
    }
    IrReceiver.resume();  // Prepare for the next signal
    delay(100);  // Optional: avoid flooding
  }
}

// -----------------------------------------------------------------

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
  if(digitalRead(ledPinIndicator) == HIGH){
    earlyShot = false;
  }else{
    earlyShot = true;
  }
}
