// To use:
// 1. Copy Setup105_RP2040_ILI9341_SPI.h to \User_Setups\
// 2. Add to User_Setup.Select.h:
//    #include <User_Setups/Setup105_RP2040_ILI9341_SPI.h> // My RP2040 setup

// Stephen X. Zhang 2024

#include "FS.h"

#include <SPI.h>
#include <TFT_eSPI.h>      // Hardware-specific library

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
#define TFT_GREY 0x5AEB

// Debug
#define debugmode false

// Frame
#define w 250 // Width
#define h 200 // Height
#define fx 10 // Origin x
#define fy 20 // Origin y
#define yinc h/5 // Y tick increment

// Invoke the TFT_eSPI button class and create all the button objects
TFT_eSPI_Button key[5];

// Using two fonts since numbers are nice when bold
#define LABEL1_FONT &FreeSansOblique9pt7b // Key label font 1
#define LABEL2_FONT &FreeSansBold12pt7b    // Key label font 2
char keyLabel[5][5] = {"AI0", "AI1", "AI2", "AI3", "STOP"};
uint16_t keyColor[5] = {TFT_VIOLET, TFT_VIOLET, TFT_VIOLET, TFT_VIOLET, TFT_RED};

// Channel
uint8_t chsel = 0;
bool chbusy = false; // picodaq busy
bool datastreaming = true; // Requesting data from picodaq or not

// Data
#define maxind 250
uint8_t data[maxind];
bool redrawtitle = true; // Redraw title
bool redrawdata = true; // Redraw data
bool pausedata = false;
bool scaledata = false; // 

// Timers
unsigned long int td, td2; // Debug
unsigned long int t0, tswitch;
unsigned long int tblind = 1100; // How long to stay blind after switching channels (to allow sync)


// Touch
#define applycal true
uint16_t calData[5] = {400, 3500, 320, 3430, 1};

// Debug serial
byte m, n;

// I2c
#include <Wire.h>
const byte i2c_address = 1;
byte m_i2c, n_i2c;
const byte handshake[2] = {211, 44}; // Arbitrary handshake signal. If no data inbetween, it means busy


void setup() {
  Serial.begin(9600); // For debug
  
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  // Apply calibration
  #if applycal
    tft.setTouch(calData);
  #endif
  
  // Draw frame
  drawframetext();
  wipeframe();

  // Draw title
  drawtitle();

  // Draw buttons
  drawbuttons();

  // Initiate and draw data
    debugdatagen(data);
//  debugdatagen2(data);
  dataplot(data);

}

void loop() {
  // put your main code here, to run repeatedly:
  t0 = millis();
  
  // Get Serial in
  if (Serial.available() >= 2) {
    // Read 2 bytes
    m = Serial.read();
    n = Serial.read();

    // Parse serial
    parseserial();
  }

  // Touch
  touch_detect();

  // ======================= redraw title =========================
  // Redraw title
  if (redrawtitle){
    redrawtitle = false;
    drawtitle();
  }

  // Pause data (right after channel switch)
  if (pausedata){
    pausedata = false;
    wipeframe();
  }
  
  // ======================= redraw data =========================
  if ((t0 - tswitch) > tblind){
    // Redraw data
    if (redrawdata){
      redrawdata = false;
      if ((chbusy) || (!datastreaming)){
        // Data no good
        datanogood();
      }
      else{
        wipeframe();
        dataplot(data);
      }
    }
  }
}
