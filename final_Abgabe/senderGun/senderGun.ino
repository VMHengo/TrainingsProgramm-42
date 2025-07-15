#include <IRremote.h>

#define PLAYER 1 // Hier setzten für welchen Player der Code gilt (1 Orange, 2 Blau)

const int irLedPin = 2;
const int speakerPin = 6;  // Pin der PWM kann
const int buttonPin = 4;
const int laserPin = 3;
const int motorPin = 5; // Gemeint ist der VibrationsMotor

const int vibrationStrength = 150;
const int feedbackDuration = 150; //in ms

uint32_t message = 0x00000001; // Feste Bit Sequenz die Spieler 1 abfeuert

bool triggerReleased = true; // Wurde der Trigger losgelassen

int currentBullet = 0; // An welchem Schuss  befinden wir uns gerade

const int chords[5][5] = { // Für die einzelnen Schüsse
  {440, 622, 0, 0, 0},       // Schuss 1: A + D# 
  {392, 523, 622, 0, 0},     // Schuss 2: G + C + D# 
  {349, 494, 587, 659, 0},   // Schuss 3: F + B + D + E 
  {330, 440, 554, 622, 698}, // Schuss 4: E + A + C# + D# + F
  {311, 392, 466, 523, 622}  // Schuss 5: D# + G + A# + C + D# 
};

const int chordLengths[5] = {2, 3, 4, 5, 5}; // Anzahl Töne pro Akkord
const int chordTotalDuration = 200;  // Dauer jedes Akkords
const int pauseBetweenShots = 150;

uint16_t sigA[] = {900, 450, 560, 560};  // längeres zweites ON

IRsend irsend;


void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(laserPin, OUTPUT);
  pinMode(motorPin, OUTPUT);
  IrSender.begin(irLedPin);  // Start von IR-Transmitter an pin 8
}

// Spielt den Akkord der freqs übergeben wurde, count stellt sicher, dass im Endeffekt alle verschiedneen AKkorde geleiche länge haben (aber ein Ton ggf unterschiedlich lang ist)
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

// Fester Chord, der Signalisiert das die Gun Ready ist nach dem Reloaden
void playReadyChord() {
  int readyChord[] = {261, 329, 392};
  for (int i = 0; i < 3; i++) {
    tone(speakerPin, readyChord[i]);
    delay(70);
    noTone(speakerPin);
  }
}

// SPielt unseren Reloadsound ab
void playReloadSound() {
  for (int f = 300; f <= 1500; f += 25) {
    tone(speakerPin, f);
    delay(15);
  }
  noTone(speakerPin);
  playReadyChord();
}

// Macht den Laser und den Motor für "ms" millisekunden an
void feedbackShot(int ms){
  digitalWrite(laserPin, HIGH);
  analogWrite(motorPin, vibrationStrength);
  delay(ms);
  analogWrite(motorPin, 0);
  delay(100);
  digitalWrite(laserPin, LOW);
}

void loop() {
  if ((digitalRead(buttonPin) == HIGH) && triggerReleased) { //Falls wir ein Signal vom Button erhalten und der trigger losgelassen wurde
    triggerReleased = false;
    if(PLAYER == 1){
      IrSender.sendNEC(message, 32);
    }else if(PLAYER == 2){
      IrSender.sendRaw(sigA, sizeof(sigA), 38);
    }
    
    feedbackShot(feedbackDuration);
    
    playChord(chords[currentBullet], chordLengths[currentBullet]);

    currentBullet = (currentBullet + 1) % 5;
  }else if ((digitalRead(buttonPin) == LOW) && !triggerReleased){ // Falls der Button losgelassen wurde und der Trigger gedrückt wurde
    triggerReleased = true;
    if(currentBullet == 0){ // Reloading Wird ausgeführt, wenn wir durch das ganze Magazin (5 Schuss) gegangen sind
      delay(100);
      playReloadSound();
    }
  }
}