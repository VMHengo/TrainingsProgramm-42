// VERSION: 5.7.2025 3:36
#include <Wire.h>
#include <Adafruit_Arcada.h>
#include <Adafruit_NeoPixel.h>
#include "sam.h"
#include <math.h>

#define LED_PIN    5. // Pin an der unser LED-Strip angeschlossen ist
#define LED_COUNT 10 // Anzahl der LEDs an unserem Strip

#define SLAVES_ANZAHL 5 // Anzahl wie viele Slaves wir haben
#define GAMEMODE_ANZAHL 2 // Anzahl wie viele GameModes es gibt
uint8_t slaves[SLAVES_ANZAHL] = {0x10, 0x20, 0x30, 0x40, 0x50}; // Addressen der Slaves
uint8_t gameModes[GAMEMODE_ANZAHL] = {0x10, 0x20}; // Addressen der Slaves die einen GameMode haben
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#define SFX_COUNT 6 // Wie viele SFX-Files wir haben
Adafruit_Arcada arcada;
File sfxFile;
volatile bool playing = false; // Ob ein SFX file gerade abspielt

#define RST_BTN_PIN // Pin auf dem unserer Reset-Button ist


bool inMenu = true; // Zustand ob wir im Menü sind
uint8_t gameMode = 0; // Zustand in welchem Game wir uns befinden


// Inititalisiere alle Objekte für unseren LED-Strip und setzte die Pins in den richtigen Modus
void setup() {
  Serial.begin(9600);

  if (!arcada.arcadaBegin())
  {
    errorReporter("Fatal error: Arcada library failed.");
  }
  if (!arcada.filesysBegin()) { 
    errorReporter("Fatal error: Filesystem failed.");
  }

  Wire.begin();
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  pinMode(RST_BTN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RST_BTN_PIN), resetAll, FALLING); // Der Button soll ein Interrupt auslösen
  delay(2000);
}


void loop() {
  if(inMenu){
    handleMenu();
  }else if(gameMode == 0x10){
    gameMode1();
    gameMode = 0;
  }else if(gameMode == 0x20){
    gameMode2();
    gameMode = 0;
  }else{
    //Serial.println("BIG ERROR");
  }
}

// Logik für unser Menü
void handleMenu(){
  gameMode = 0;
  for(int i=0; i < GAMEMODE_ANZAHL; i++){ // Sende an die Slaves mit einem GameMode den Befehl ins Menü zu gehen
    Wire.beginTransmission(gameModes[i]);
    Wire.write(1);
    Wire.endTransmission();
  }
  resetScoreboard();
  loadSFX(4);
  gameMode = requestGameModeFromSlaves(); // Erhalte den GameMode der ausgewählt wurde
  Serial.println(gameMode);
  Wire.beginTransmission(gameMode);
  Wire.write(3);  // Code 3 = Spiele Animation von Auswahl ab
  Wire.endTransmission();
  loadSFX(3);
  delay(1800); // Zeit der LED Animation
  broadcastCmd(2); // Setzte alle Slaves in einen sicheren zustand
  inMenu = false;
}

// Logik des ersten Spiels, es gewinnt derjenige, der zuerst 5 Punkte erreicht
void gameMode1(){
  randomSeed(millis());
  loadSFX(5);
  long slave_index;
  uint32_t currentWinner;
  int orangePoints = 0;
  int bluePoints = 0;
  int requiredPoints = 5; // HIER MIN. PUNKTZAHL ZUM GEWINNEN ÄNDERN
  while(orangePoints != requiredPoints && bluePoints != requiredPoints){
    slave_index = random(0, SLAVES_ANZAHL); // Slave der zur Zielfläche werden soll
    currentWinner = activateOneTargetGM1(slaves[slave_index]); // Ermittle denjenigen der Zuerst draufgeschossen hat
    if(currentWinner == 0x00000001){
      strip.setPixelColor(orangePoints, 255, 255, 0);
      strip.show();
      orangePoints += 1;
    }else if(currentWinner == 0x00000002){
      strip.setPixelColor(9-bluePoints, 0, 0, 255);
      strip.show();
      bluePoints += 1;
    } 
  }
  if(orangePoints == requiredPoints){
    broadcastCmd(98); // Gewinn Animation für Orange
    setScoreboard(255,255,0);
  }else if(bluePoints == requiredPoints){
    setScoreboard(0,0,255); 
    broadcastCmd(99); // Gewinn Animation für Blau
  }
  loadSFX(0);
  delay(5000); // 5 Sekunden sieger Animation puffer um zu Gewährleisten, dass auf slaves Seite nichts kaputt geht
  inMenu = true;
}

// Logik des zweiten Spiels, es gewinnt derjenige, der nach 45sek mehr Punkte hat
void gameMode2(){
  randomSeed(millis());
  loadSFX(5);
  long slave_index = 0;
  uint32_t currentWinner;
  int orangePoints = 0;
  int bluePoints = 0;
  unsigned long startTime = millis(); 
  unsigned long newTimeWD = millis(); // Variable die benötigt wird, um alle 10 Sekunden timeForShot runterzusetzen
  unsigned long timeForShot = 2000; // Die Zeit die man hat um eine Zielfläche anzuschießen
  uint8_t lastSlave = 0x00; // Variable um abzuspeichern wlcher Slave zuletzt eine Zielfläche war
  while((startTime + 45000) >= (millis()) || orangePoints == bluePoints){ // Schleife geht solange bis es einen Gewinner gibt UND mind. 45sek um sind
    if(newTimeWD + 10000 <= millis()){ // Logik um alle 10 sekunden timeForShot runterzusetzen
      if((startTime + 45000) >= (millis())){
        timeForShot -= 250;
        newTimeWD = millis();
      }
    }
    do{
      slave_index = random(0, SLAVES_ANZAHL); 
    }while((uint8_t) slave_index == lastSlave); // Wähle solange einen neues Ziel (Slave) aus, bis er nicht mehr der gleiche ist wie unser letztes Ziel
    lastSlave = (uint8_t) slave_index;

    currentWinner = activateOneTargetGM2(slaves[slave_index], timeForShot); // Erhalte denjenigen der auf die Zielfläche zuerst geschossen hat (wenn überhaupt)
    if(currentWinner == 0x00000001){
      orangePoints += 1;
    }else if(currentWinner == 0x00000002){
      bluePoints += 1;
    }
    scoreBoardGM2(orangePoints, bluePoints); // Update das Scoreboard
  }
  if(orangePoints>bluePoints){
    setScoreboard(255,255,0);
    broadcastCmd(98); // Gewinn Animation für Orange
  }else if(bluePoints > orangePoints){
    setScoreboard(0,0,255);
    broadcastCmd(99); // Gewinn Animation für Blau
  }
  loadSFX(0);
  delay(5000); // 5 Sekunden sieger Animation puffer um zu Gewährleisten, dass auf slaves Seite nichts kaputt geht
  inMenu = true;
}

// Sendet den command (cmd) an jeden einzelnen Slave in unserem "slaves" array
void broadcastCmd(int cmd){
  for(int i=0; i < SLAVES_ANZAHL; i++){
    Wire.beginTransmission(slaves[i]);
    Wire.write(cmd);
    Wire.endTransmission();
  }
}

// Frage die Slaves nacheinander ab auf wen geschossen wurde, derjenige der getroffen wurde gibt seine Adresse. Diese Addresse fungiert als gameMode, es werden nur gameModes akzeptiert die im entsprchenden gameModes Array sind
// ACHTUNG SLAVES DÜRFEN NUR ADRESSEN DER FORM 0xZZ haben, sonst funktioniert das mit dem Casten nicht in von uint32_t zu uint8_t weil bits weggeschnitten werden
uint8_t requestGameModeFromSlaves(){
  int i = 0;
  uint32_t response = 0;
  while (!valueInGameModes((uint8_t) response)) {
    Wire.requestFrom(gameModes[i], sizeof(response));
    if (Wire.available() == 4) {
      for (int j = 0; j < sizeof(response); j++) {
        ((uint8_t *)&response)[j] = Wire.read();
      }
    }
    delay(100);
    i = (i + 1) % GAMEMODE_ANZAHL;
  }
  return (uint8_t) response;
}

// Checkt, ob "value" im gameModes Array ist
bool valueInGameModes(uint8_t value) {
  for (int i = 0; i < GAMEMODE_ANZAHL; i++) {
    if (gameModes[i] == value) {
      return true;
    }
  }
  return false;
}

// Ein einzelner Slave leuchtet grün auf, wir warten bis dieser angeschossen wurde, dann erhalten wir als rückgabewert, wer auf diesen geschossen hat
uint32_t activateOneTargetGM1(uint8_t slave){
  delay(1700 + random(0, 2000)); // Nach 1.7-3.7 Sekunden soll das Target aktiv werden
  Wire.beginTransmission(slave);
  Wire.write(10);  // Code 10 = Start von GameMode1
  Wire.endTransmission();

  uint32_t response = 0;
  while(response != 0x00000001 && response != 0x00000002 && response!= 0x00000003){ 
    Wire.requestFrom(slave, sizeof(response));
    if(Wire.available() == 4){
      for(int i=0; i<sizeof(response); i++){
        ((uint8_t *)&response)[i] = Wire.read();
      }
    }
    delay(200);
    Serial.println(response);
  }
  Wire.beginTransmission(slave);
  Wire.write(3);  // Code 3 = Animation
  Wire.endTransmission();
  loadSFX(1);
  delay(1800); // LED ANIMATION TIME, garantiert, dass die sachen nicht out of sync sind (i.e. nächster cmd wird gesendet bevor led animation fertig)
  return response;
}

// Ein einzelner Slave leuchtet grün auf, dieser muss in "givenTime" angeschossen werden, dann erhalten wir als rückgabewert wer auf diesen geschossen hat, sonst geht der Slave wieder aus und es gibt keinen Gewinner
uint32_t activateOneTargetGM2(uint8_t slave, unsigned long givenTime){
  Wire.beginTransmission(slave);
  Wire.write(22);  // Code 2 = Start von GameMode2 
  Wire.endTransmission();

  unsigned long startTime = millis();
  uint32_t response = 0;

  while(response != 0x00000001 && response != 0x00000002 && response!= 0x00000003 && (startTime + givenTime) >= (millis())){ 
    Wire.requestFrom(slave, sizeof(response));
    if(Wire.available() == 4){
      for(int i=0; i<sizeof(response); i++){
        ((uint8_t *)&response)[i] = Wire.read();
      }
    }
    delay(200);
  }

  if(response == 0x00000001 || response == 0x00000002 || response == 0x00000003){ // Nur wenn einer drauf geschossen hat wird dies Ausgeführt
    Wire.beginTransmission(slave);
    Wire.write(3);  // Code 3 = Animation
    Wire.endTransmission();
    loadSFX(1);
    delay(50); // LED ANIMATION TIME, garantiert, dass die sachen nicht out of sync sind (i.e. nächster cmd wird gesendet bevor led animation fertig)
  }

  Wire.beginTransmission(slave);
  Wire.write(23); // Resette den slave in einen Safe-State (aber noch im Zustand von GameMode2)
  Wire.endTransmission();
  return response;
}

// Setzte den Ganzen Strip aif die Farbe (r,g,b)
void setScoreboard(int r, int g, int b){
  for(int i=0; i<LED_COUNT; i++){
      strip.setPixelColor(i, r, g, b);
  }
  strip.show();
}

// Mache den ganzen Strip aus
void resetScoreboard(){
  setScoreboard(0,0,0);
}

// Spiele die SFX-File ab mit dem index
void loadSFX(int index) {
  index = (SFX_COUNT -1) - index; // index shift
  sfxFile = arcada.openFileByIndex("/sfx", index, O_READ, "wav");
  if (!sfxFile) {
    errorReporter("Fatal error: No sound file present.");
  }

  Serial.println("Loading sound file...");
  
  uint32_t sampleRate;

  // load wav information, enable speakers and set interrupt for sampling
  wavStatus status = arcada.WavLoad(sfxFile, &sampleRate);
  if ((status == WAV_LOAD) || (status == WAV_EOF)) {
    arcada.enableSpeaker(true);
    // create own interrupt
    arcada.timerCallback(sampleRate, wavOutCallback);
  } else {
    errorReporter("Fatal error: Sound file corrupted.");
  }

   do { // Repeat this loop until WAV_EOF or WAV_ERR_*
    if (arcada.WavReadyForData()) {
      yield();
      status = arcada.WavReadFile();
    }
    yield();
  } while ((status == WAV_OK) || (status == WAV_LOAD));

  // Audio might be continuing to play at this point!
  while (playing)   yield();
  // now we're really done
  sfxFile.close();
  Serial.println("Ende Sfx");
}

// Hilfsfunktion für die loadSFX funktion
void wavOutCallback(void) {
  wavStatus status = arcada.WavPlayNextSample();
  if (status == WAV_EOF) {
    // End of WAV file reached, stop timer, stop audio
    arcada.timerStop();
    arcada.enableSpeaker(false);
    playing = false;
  } else {
    playing = true;
  }
}

// Error Reporter bei fehlern
void errorReporter(const char *error) {
  Serial.begin(9600);
  Serial.println(error);

  // busy wait to keep USB connections, etc. running
  while (true) {
    yield();
  }
}

// Funktion die aufgerufen wird, wenn der Reset-Button gedrückt wird. Alle Slaves und der Pybadge werden resettet
void resetAll(){
  setScoreboard(255,0,0);
  resetSlaves();
  NVIC_SystemReset();
}

// Generische funktion
void test(){
  Serial.println("Button wurde gedrückt!");
}

// Logik um unseren Scoreboard in GameMode2 zu setzen. Die Punkte werden Prozentual zur Gesammtpunktzahl angezeigt
void scoreBoardGM2(int pointsOrange, int pointsBlue){
  int totalPoints = pointsOrange + pointsBlue;
  if(pointsOrange > 0 && pointsBlue == 0){
    setScoreboard(255,255,0);
  }else if(pointsBlue > 0 && pointsOrange == 0){
    setScoreboard(0,0,255);
  }else if(pointsOrange > pointsBlue){
    int percentageBlue = round(10 * ((float)pointsBlue / totalPoints));
    int percentageOrange = round(10 * ((float)pointsOrange / totalPoints));
    for(int i=0; i < percentageBlue; i++){
      strip.setPixelColor(9-i, 0, 0, 255);
    }
    for(int i=0; i < percentageOrange; i++){
      strip.setPixelColor(i, 255, 255, 0);
    }
  }else if(pointsOrange != 0 && pointsBlue != 0){
    int percentageBlue = round(10 * ((float)pointsBlue / totalPoints));
    int percentageOrange = round(10 * ((float)pointsOrange / totalPoints));
    for(int i=0; i < percentageOrange; i++){
      strip.setPixelColor(i, 255, 255, 0);
    }
    for(int i=0; i < percentageBlue; i++){
      strip.setPixelColor(9-i, 0, 0, 255);
    }
  }
  strip.show();
}

// Resette alle Slaves
void resetSlaves(){
  for(int i=0; i < SLAVES_ANZAHL; i++){
    Wire.beginTransmission(slaves[i]);
    Wire.write(100); // Code 100 = Slave soll sich resetten
    Wire.endTransmission();
    delay(250);
  }
}