/**
 * 
 * Our main file
 * 
 */
 
// We need the Adafruit Arcada Library for our basic functionality
#include <Adafruit_Arcada.h>
// t2: include Neopixel library
#include <Adafruit_NeoPixel.h>
// We can also include our own files in the project
#include "images.h"

#include <vector>

// t2: Neopixels are attached to PIN D8
#define PIN 8 

// t2: How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 5

// Global variables
Adafruit_Arcada arcada;

File musicFile;
bool musicIsPlaying = false;
uint32_t bpmTiming = 400;
uint32_t beatTime;

int32_t timingOffset;
bool buttonPressedForLastBeat;
bool buttonReleasedForLastBeat;

int32_t offsetX = 0;
int32_t offsetY = 0;
uint8_t directionOffset = 0;
uint8_t beatDirection = 0;
bool directionUpdate = false;

bool inMenu = true;
bool easyMode = true;
bool menuButtonReleased = true;
bool leftButtonReleased = true;
bool rightButtonReleased = true;

uint8_t pixelBrightness = 2;
std::vector<uint8_t> brightnessLevels = {5, 10, 30, 60, 100, 150, 255};

// t2: initialize pixels object
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Color constants
#define NEO_RED       pixels.Color(150, 0, 0)
#define NEO_GREEN     pixels.Color(0, 150, 0)
#define NEO_BLUE      pixels.Color(0, 0, 150)
#define NEO_YELLOW    pixels.Color(255, 255, 0)


/**
 * The Arduino setup function (called once on startup)
 */
void setup() {
  // initialize arcada library
  if (!arcada.arcadaBegin())
  {
    errorReporter("Fatal error: Arcada library failed.");
  }

  // initialize the file system on the flash (first flashing should be done via CircuitPython)
  if (!arcada.filesysBegin()) { // arcada.filesysBeginMSD() for "USB-Stick" mode
    errorReporter("Fatal error: Filesystem failed.");
  }
  
  // initialize serial connection
  Serial.begin(9600);
  Serial.println("Booting into game...");
  
  // configure board display and enable frame buffer mode
  arcada.displayBegin();
  arcada.setBacklight(255);
  arcada.createFrameBuffer(ARCADA_TFT_WIDTH, ARCADA_TFT_HEIGHT);

  // t2: INITIALIZE NeoPixel strip object
  pixels.begin(); 

  pixels.setBrightness(pixelBrightness); // t2: set Neopixel brightness between 0 and 255

  Serial.println("Initializing neopixels...");

  // we start our loop in the menu
  inMenu = true;
  
  Serial.println("Finished booting!");
}


/**
 * The Arduino loop function (called repeatedly)
 */
void loop() {

  // t2: set all pixels to off
  pixels.clear();

  // let's just pretend it's a state machine
  if (inMenu) {
    handleMenu();
  } else {
    handleGameplay();
  }

  // render the frame buffer to the screen
  arcada.blitFrameBuffer(0, 0, true, false);

  // t2: Send the updated pixel colors to the hardware.
  pixels.show();

  // perform standard board functions (cooperative multitasking)
  yield();
}


/**
 * Handle menu input and rendering
 */
void handleMenu() {
  uint8_t buttonState = arcada.readButtons();
  // Serial.println(buttonState);
  GFXcanvas16 *canvas = arcada.getCanvas();
  // start pressed: load music and switch to game state
  if (buttonState & ARCADA_BUTTONMASK_START) {
    loadMusic();
    randomSeed(millis());
    inMenu = false;
  }

  // select pressed: toggle difficulty
  if (buttonState & ARCADA_BUTTONMASK_SELECT) {
    if (menuButtonReleased) {
      menuButtonReleased = false;
      easyMode = !easyMode;
    }
  } else {
    menuButtonReleased = true;
  }

  // brightness setting in menu
  if (buttonState & ARCADA_BUTTONMASK_LEFT) {
    if (leftButtonReleased) {
      leftButtonReleased = false;
      if (pixelBrightness != 1) {
        pixelBrightness = pixelBrightness >> 1;
        pixels.setBrightness(pixelBrightness);
      }
    }
  } else {
    leftButtonReleased = true;
  }

  if (buttonState & ARCADA_BUTTONMASK_RIGHT) {
    if (rightButtonReleased) {
      rightButtonReleased = false;
      if (pixelBrightness < 128) {
        pixelBrightness = pixelBrightness << 1;
        pixels.setBrightness(pixelBrightness);
      }
    }
  } else {
    rightButtonReleased = true;
  }

  // use standard rendering functions without directly updating the screen
  canvas->fillScreen(ARCADA_WHITE);
  
  canvas->setTextSize(2);
  canvas->setTextColor(ARCADA_BLUE);
  canvas->setCursor(2, 2);
  canvas->print("Not your");
  canvas->setTextColor(ARCADA_PURPLE);
  canvas->setCursor(18, 18);
  canvas->print("Ordinary");
  canvas->setTextColor(ARCADA_RED);
  canvas->setCursor(34, 34);
  canvas->print("Percussion");
  canvas->setTextColor(ARCADA_ORANGE);
  canvas->setCursor(50, 50);
  canvas->print("Escapade");

  if (easyMode) {
    canvas->drawRGBBitmap(ARCADA_TFT_WIDTH/2 - 32, ARCADA_TFT_HEIGHT - 64, drum_easy, 64, 64);
  } else {
    canvas->drawRGBBitmap(ARCADA_TFT_WIDTH/2 - 32, ARCADA_TFT_HEIGHT - 64, drum_hard, 64, 64);
  }
  // TO DO: Update Neopixels based on difficulty
  if (easyMode) {
    pixels.fill(NEO_GREEN);
  } else {
    pixels.fill(NEO_RED);
  }

}


/*
 * Handle game rendering and partial input
 */
void handleGameplay() {
  // music main loop
  if (musicIsPlaying) {
    // fill music buffer if needed
    if (arcada.WavReadyForData()) {
      wavStatus status = arcada.WavReadFile();
    }
  } 

  // handle button release (did that here because in the interrupt buttons sometimes don't seem to be debounced...)
  uint8_t buttonState = arcada.readButtons();
 
  if (!(buttonState & ARCADA_BUTTONMASK_A)) {
    buttonReleasedForLastBeat = true;
  }

  // render main loop
  uint32_t timeNow = millis();

  // -----------------------
  // t2: Main Game Loop
  // -----------------------

  // t2: fill screen black after selecting difficulty
  GFXcanvas16 *canvas = arcada.getCanvas();
  canvas->fillScreen(ARCADA_BLACK);

  // calculate pixel beat indicator position
  int32_t beatDiff = abs((int32_t)(beatTime - timeNow));
  int32_t pixelIndex = map(beatDiff, 0, 175, 0, 3);
  
  pixels.setPixelColor(pixelIndex, NEO_BLUE);

  // render indicator circle if needed
  if (timeNow - beatTime > 0 && timeNow - beatTime < 100) {
    if (buttonPressedForLastBeat) {
      if (abs(timingOffset) < 50) {
        // good
        pixels.setPixelColor(0, NEO_GREEN);
      } else if (abs(timingOffset) < 90) {
        // barely ok
        pixels.setPixelColor(0, NEO_YELLOW);
      } else {
        // bad
        pixels.setPixelColor(0, NEO_RED);
      }
    } else {
      // missed a beat and hit the button before the next beat occured
      pixels.setPixelColor(0, NEO_RED);
    }
  } //else {
  //   pixels.setPixelColor(0, NEO_BLUE);
  // }
}


/*
 * Prepares the PyBadge to play music
 */
void loadMusic() {
  // open the first wav file in the music folder
  musicFile = arcada.openFileByIndex("/music", 0, O_READ, "wav");
  if (!musicFile) {
    errorReporter("Fatal error: No sound file present.");
  }

  Serial.println("Loading sound file...");
  
  uint32_t sampleRate;

  // load wav information, enable speakers and set interrupt for sampling
  wavStatus status = arcada.WavLoad(musicFile, &sampleRate);
  if ((status == WAV_LOAD) || (status == WAV_EOF)) {
    arcada.enableSpeaker(true);
    // create own interrupt
    arcada.timerCallback(sampleRate, timedInterruptCallback);
  } else {
    errorReporter("Fatal error: Sound file corrupted.");
  }
}


/*
 * Minimal error reporter (adapted from Adafruit examples - without the LED blinking stuff)
 */
void errorReporter(const char *error) {
  Serial.begin(9600);
  Serial.println(error);

  // busy wait to keep USB connections, etc. running
  while (true) {
    yield();
  }
}


/*
 * Interrupt for music sampling and game updates (called with 8kHz on our wav file - other sampling rates might not work)
 */
void timedInterruptCallback(void) {
  // we just entered the playing state
  // move first beat until music actually starts
  if (!musicIsPlaying) {
    beatTime = millis() + bpmTiming;
  }

  // play next sample
  wavStatus status = arcada.WavPlayNextSample();

  // update timing information
  uint32_t timeNow = millis();

  // if needed prepare calculations for next beat
  // called when beat missed
  if (timeNow >= beatTime + (bpmTiming>>1)) {
    beatTime = beatTime + bpmTiming;
    buttonPressedForLastBeat = false;
    beatDirection = directionOffset;
    directionUpdate = true;
  }

  // make sure that circle expansion direction changes according to beats
  if (directionUpdate && timeNow >= beatTime) {

    if (easyMode) {
      directionOffset = directionOffset + 1;
    } else {
      directionOffset = random(4);
    }
    
    if (directionOffset > 3) {
      directionOffset = 0;
    }
    
    directionUpdate = false;
  }

  // handle buttons
  uint8_t buttonState = arcada.readButtons();

  // a button always registers beat tap
  if (buttonState & ARCADA_BUTTONMASK_A) {
    if (!buttonPressedForLastBeat && buttonReleasedForLastBeat) {
      timingOffset = beatTime - timeNow;
      buttonPressedForLastBeat = true;
      buttonReleasedForLastBeat = false;
    }
  }

  // direction pad only registers beat tap in the right direction
  if (!buttonPressedForLastBeat && buttonReleasedForLastBeat && buttonState) {
    switch (beatDirection){
      case 0:
        if (buttonState & ARCADA_BUTTONMASK_RIGHT) {
          timingOffset = beatTime - timeNow;
        } else {
          timingOffset = bpmTiming>>1;
        }
        break;
      case 1:
        if (buttonState & ARCADA_BUTTONMASK_UP) {
          timingOffset = beatTime - timeNow;
        } else {
          timingOffset = bpmTiming>>1;
        }
        break;
      case 2:
        if (buttonState & ARCADA_BUTTONMASK_LEFT) {
          timingOffset = beatTime - timeNow;
        } else {
          timingOffset = bpmTiming>>1;
        }
        break;
      case 3:
        if (buttonState & ARCADA_BUTTONMASK_DOWN) {
          timingOffset = beatTime - timeNow;
        } else {
          timingOffset = bpmTiming>>1;
        }
        break;
      default:
        break;
    }

    buttonPressedForLastBeat = true;
    buttonReleasedForLastBeat = false;
  }

  // update playing state and hardware depending on wav playing status
  if (status == WAV_EOF) {
    // end interrupt
    arcada.timerStop();
    arcada.enableSpeaker(false);
    musicIsPlaying = false;
    inMenu = true;
    // close file handle
    musicFile.close();
  } else {
    musicIsPlaying = true;
  }
}
