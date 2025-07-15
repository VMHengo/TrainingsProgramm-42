// VERSION: 5.7.2025 3:36
#include <Arduino.h>
#include <Wire.h>
#include <IRremote.h>
#include <string>

#define IR_RECEIVE_PIN 2
#define I2C_ADDR 0x50 // HIER DIE ADDRESSE FÜR DEN AKTUELLEN SLAVE ÄNDERN

#define redPin 3 // Red Input für die RGB
#define greenPin 4 // Green Input für die RGB
#define bluePin 5 // Blue Input für die RGB

int gameMode = 0; // GameMode 0 = bedeutet kein Game bzw. im Menü
bool waitingForShot = false; // Wir wollen gerade keinen schuss erhalten
uint32_t hexCode = 0xBBBBBBBB; // Feste Bitsequenz, die bei den while-loops im Master immer dafür sorgen soll, dass die Schleifenbedingung nicht gilt (also niemals nach diesem Code Fragen)

void setup() {
  Serial.begin(9600);
  Wire.begin(I2C_ADDR);
  Wire.onReceive(receiveEvent); 
  Wire.onRequest(requestEvent);

  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  setColor(0, 0, 0);
}

void loop() {
  if(waitingForShot && gameMode == 0 && IrReceiver.decode()){   // Logik fürs Menu
    if(fixBits(IrReceiver.decodedIRData.decodedRawData) == 0x00000001){
      waitingForShot = false; 
      hexCode = I2C_ADDR; // HexCode ERST NACHDEM waitingForShot AUF FALSE GESETZT WURDE setzen
    }else{
      delay(50);
      IrReceiver.resume();
    }
  } 
  else if(waitingForShot && gameMode == 1 && IrReceiver.decode()) { // Logik für den ersten gameMode
    uint32_t receivedCode = fixBits(IrReceiver.decodedIRData.decodedRawData);
    Serial.println(receivedCode);
    if(receivedCode == 0) {
      waitingForShot = false;
      hexCode = 0x00000002; // HexCode ERST NACHDEM waitingForShot AUF FALSE GESETZT WURDE setzen
    }else{
      waitingForShot = false;
      hexCode = 0x00000001;
    }
  }else if(waitingForShot && gameMode == 2 && IrReceiver.decode()){ // Logik für den zweiten gameMode
    uint32_t receivedCode = fixBits(IrReceiver.decodedIRData.decodedRawData);
    if(receivedCode == 0) {
      waitingForShot = false;
      hexCode = 0x00000002; // HexCode ERST NACHDEM waitingForShot AUF FALSE GESETZT WURDE setzen
    }else{
      waitingForShot = false;
      hexCode = 0x00000001;
    }
  }
}   

// Funktion die ausgeführt wird, wenn der Master einen Cmd schickt
void receiveEvent(int howMany){
  if(howMany > 0){
    byte cmd = Wire.read();
    switch (cmd){
      case 1: // MENU
        resetVars(0,true); // Hilfsfunktion um die Variablen schnell zu setzen
        setColor(255,0,255); // lila
        IrReceiver.resume(); // Wir flushen den Buffer
        break;
      case 2: // LED aus und Slave soll kein Schuss aufnehmen können
        resetVars(0,false);
        setColor(0,0,0);
        IrReceiver.resume();
        break;
      case 3: // Logik für die Animationen (unterschiedlich für unterschiedliche gameModes)
        if(hexCode == 0x00000001 && gameMode ==1){
          animationLED('y');  // Yellow
        }else if(hexCode == 0x00000002 && gameMode == 1){
          animationLED('b');  // Blue
        }else if(hexCode == 0x00000003 && gameMode == 1){ 
          animationLED('r');  // Blue
        }else if((hexCode == 0x10 || hexCode == 0x20) && gameMode == 0){
          animationLED('t');  // Blue
        }else if(hexCode == 0x00000001 && gameMode == 2){
          setColor(255,255,0);
        }else if(hexCode == 0x00000002 && gameMode == 2){
          setColor(0,0,255);
        }else if(hexCode == 0x00000003 && gameMode == 2){
          setColor(255,0,0);
        }else{
          setColor(255,0,0);
        }
        break; 
      case 10: // GameMode 1
        resetVars(1,true);
        setColor(0,255,0); // grün
        IrReceiver.resume(); // Wir flushen den Buffer 
        break;
      case 22: // GameMode 2 (target an)
        resetVars(2,true);
        setColor(0,255,0);
        IrReceiver.resume(); // Wir flushen den Buffer 
        break;
      case 23: // GameMode 2 (target aus)
        resetVars(2,false);
        setColor(0,0,0);
        IrReceiver.resume();
        break;
      case 98: // Winner Animation Orange
        resetVars(0,false);
        IrReceiver.resume(); // Wir flushen den Buffer
        blinkingLED(255,255,0, 5, 500);
        break;
      case 99: // Winner Animation Blue
        resetVars(0,false);
        IrReceiver.resume(); // Wir flushen den Buffer
        blinkingLED(0,0,255, 5, 500);
        break;
      case 100: // Reset des Slaves
        NVIC_SystemReset();
        break;
    } 
  }
}

// Funktion die ausgeführt wird, wenn der Master vom Slave etwas Requested
void requestEvent() {
  Wire.write((uint8_t*)&hexCode, sizeof(hexCode));
}

// Dreht die Bit-Sequenz des hexcodes einmal um (sonst ist es andersherum, wie es im Waffencode aufzufinden ist)
uint32_t fixBits(uint32_t val) {
  uint32_t result = 0;
  for (int i = 0; i < 32; i++) {
    result <<= 1;
    result |= (val & 1);
    val >>= 1;
  }
  return result;
}

// Setzt die Farbe der RGB-LED
void setColor(int r, int g, int b) {
  analogWrite(redPin, 255 - r);
  analogWrite(greenPin, 255 - g);
  analogWrite(bluePin, 255 - b);
}

// r,g,b setzen die Farben, k sagt wie viel mal es aufblinken soll, d sagt uns wie schnell es blinken soll
void blinkingLED(int r, int g, int b, int k, unsigned long d){
  for (int i = 0; i<k; i++){
    setColor(r,g,b);
    delay(d);
    setColor(0,0,0);
    delay(d);
  }
}

// Feste Animationen die Schnell aufgerufen werden können
void animationLED(char color){
  switch (color) {
    case 't': // Türkis
      blinkingLED(0,204,204, 3, 300);
      break;
    case 'r':
      blinkingLED(255,0,0, 3, 300);
      break;
    case 'y':
      blinkingLED(255,255,0, 3, 300);
      break;
    case 'b':
      blinkingLED(0,0,255, 3, 300);
      break;
    case 'g':
      blinkingLED(0,255,0, 3, 300);
      break;
  }
}

// Funktion um Schnell die wichtigen Logikvariablen zu setzen
void resetVars(int pGameMode, bool pWaitingForShot){
  gameMode = pGameMode;
  waitingForShot = pWaitingForShot;
  hexCode = 0xBBBBBBBB;
}
