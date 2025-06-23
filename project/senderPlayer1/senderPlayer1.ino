#include <IRremote.h>

const int irLedPin = 2;
const int speakerPin = 6;  // Use a PWM-capable pin
const int buttonPin = 4;
const int laserPin = 3;
const int motorPin = 5;

const int vibrationStrength = 150;
const int feedbackDuration = 100; //in ms

uint32_t message = 0xB1B1B1B1;

bool triggerReleased = true;

int currentBullet = 0;

const int chords[5][5] = {
  {440, 622, 0, 0, 0},       // Schuss 1: A + D# (Tritonus)
  {392, 523, 622, 0, 0},     // Schuss 2: G + C + D# (düster, halbdissonant)
  {349, 494, 587, 659, 0},   // Schuss 3: F + B + D + E (dissonant cluster)
  {330, 440, 554, 622, 698}, // Schuss 4: E + A + C# + D# + F (spannend)
  {311, 392, 466, 523, 622}  // Schuss 5: D# + G + A# + C + D# (fast atonal)
};

const int chordLengths[5] = {2, 3, 4, 5, 5}; // Anzahl Töne pro Akkord
const int chordTotalDuration = 200;  // Dauer jedes Akkords
const int pauseBetweenShots = 150;


IRsend irsend;


void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(laserPin, OUTPUT);
  pinMode(motorPin, OUTPUT);
  IrSender.begin(irLedPin);  // Start IR transmitter on pin 8
}

void playChord(const int* freqs, int count) {
  int singleDuration = chordTotalDuration / count;
  for (int i = 0; i < count; i++) {
    if (freqs[i] > 0) {
      tone(speakerPin, freqs[i]);
      delay(singleDuration);
      noTone(speakerPin);
    }
  }
}

void playReadyChord() {
  int readyChord[] = {261, 329, 392};
  for (int i = 0; i < 3; i++) {
    tone(speakerPin, readyChord[i]);
    delay(70);
    noTone(speakerPin);
  }
}

void playReloadSound() {
  for (int f = 300; f <= 1500; f += 25) {
    tone(speakerPin, f);
    delay(15);
  }
  noTone(speakerPin);
  playReadyChord();
}

void feedbackShot(int ms){
  digitalWrite(laserPin, HIGH);
  analogWrite(motorPin, vibrationStrength);
  delay(ms);
  digitalWrite(laserPin, LOW);
  analogWrite(motorPin, 0);
}

void vibrateMotor(){
  analogWrite(motorPin, vibrationStrength);
  delay(100);
  analogWrite(motorPin, 0);
}

void loop() {
  if ((digitalRead(buttonPin) == HIGH) && triggerReleased) {
    triggerReleased = false;
    IrSender.sendNEC(message, 32);  // 32-bit example code
    feedbackShot(150);
    
    playChord(chords[currentBullet], chordLengths[currentBullet]);

    currentBullet = (currentBullet + 1) % 5;
    

    //delay(400);  // Debounce delay
  }else if ((digitalRead(buttonPin) == LOW) && !triggerReleased){
    triggerReleased = true;
    if(currentBullet == 0){
      delay(100);
      playReloadSound();
    }
  }
}
