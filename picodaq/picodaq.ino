// File structure
// Here:  pico core 0 for digital data acquisition and data streaming
// 1_ADC: pico core 1 for babysitting the ADC and administrative serial communication and i2c
// 2_epr: eeprom functions
// 2_sem: serial menu
// 2_ser: serial communication functions
// ADS131M04.cpp - library cpp
// ADS131M04.cpp - library helper

// Debug
#define debugmode false

// Version
#define pdvers "v1.2"

// ===================== Digital =====================
// Digital inputs
const byte ndin = 16;
const byte dins[ndin] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 31, 32};
const byte ledpin = 25;
bool ledon = false;

// Digital data
uint32_t dnow, dnow2, dout;

// Digital out (dout0 = adc running)
const byte ndout = 2;
const byte douts[ndout] = {22, 28};

// ===================== Analog =====================
#include "ADS131M04.h"
#include "SPI.h"
ADS131M04 adc;
adcOutput res;

// Analog inputs
const byte nain = 4;
byte mux = 0; // 0 = Diff, 1 = GND, 2 = POS, 3 = NEG

// ADC pins
// clk_pin, miso_pin, mosi_pin, cs_pin, drdy_pin, reset_pin
const byte clk_pin = 18;
const byte miso_pin = 16;
const byte mosi_pin = 19;
const byte cs_pin = 17;
const byte drdy_pin = 20;
const byte reset_pin = 21;

// ADC sampling rate
byte adc_fs = 3; // 0 = 32 kSPS, 1 = 16 kSPS, 2 = 8 kSPS, 3 = 4 kSPS (default), 4 = 2 kSPS, 5 = 1 kSPS, 6 = 500 kSPS, 250 kSPS

// ADC gain
byte adc_gain = 0; // 0 = 1, 1 = 2, 2 = 4, 3 = 8 , 4 = 16, 5 = 32, 6 = 64, 7 = 128

// ADC buffer 
int32_t anow0, anow1, anow2, anow3; // Scaled adc data
int32_t aout0, aout1, aout2, aout3; // Output data (unconverted)
float af0, af1, af2, af3; // Output data (converted)
int32_t ab0[4], ab1[4], ab2[4], ab3[4]; // stores previous 4 data reads at max
byte adepthw = 0; // pointer to where to save analog data
byte adepthr = 0; // Pointer to where to grab analog data
byte adepth_max = 2; // This many analog data points will be averaged and sent out for each picodaq data upload (4 means 0 to 3)

// Send data in 24-bit two's complement (freeing up 1 byte per analog channel)
#define send24bittwos false

// ================== Analog calibration ==================
#define ncal 256
#define shiftcal 8
uint16_t ical; // iteration
int32_t cal32_0, cal32_1, cal32_2, cal32_3, cal32_e; // calibration 32 bits
uint32_t cal32_0u, cal32_1u, cal32_2u, cal32_3u, cal32_eu; // stores unsigned 32 GCAL value
uint32_t cal32_0o, cal32_1o, cal32_2o, cal32_3o, cal32_eo; // stores unsigned 32 OCAL value
uint32_t extv = 512; // Default external voltage for calibration (mV)
float extvfloat = 0.512; // Default external voltage for calibration (V)
byte chcal = 0; // Channel to calibrate
byte selcal = 0; // Selector for receiving calibration from USB

// ===================== EEPROM =====================
#include "Wire.h"
#include "I2C_eeprom.h"
I2C_eeprom ee(0x50, I2C_DEVICESIZE_24LC16);
// 0: signature
// 4, 8, 12, 16: offset cal (0x10 internal, 0x20 external)
// 20, 24, 28, 32: gain cal (0x10 internal 0x20 external)
#define eesig 97
#define eesigaddr 2
#define eeintcal 0x10
#define eeextcal 0x20
#define autoeeini true

// ===================== I2c data =====================
// Pulling additional data via I2c bus. Unclear if this will run into issues with the EEPROM code
#define i2c_data true
#define i2c_data_bytes 2 // Default 2 bytes of data
bool i2c_data_use = true;
byte i2c_dataadd = 1; // Default where to get i2c data
byte m_i2c, n_i2c;
unsigned long int ti2cdebug; // debug

// ===================== Serial =====================
// Serial
byte m, n;

// ===================== Operation =====================
// Sampling rate
uint32_t fs = 1; // In Hz
unsigned long int ts = 1000000 / fs;  

// Timers
unsigned long int tnow, tnowmillis, t0, t0millis, t1;

// Sync signal
unsigned long int tsend;
bool sync_high = false;
uint16_t syncwidth = 200; // In us

// Flags
bool pulse = false; // picodaq running
bool datagood = false; // picodaq ready to upload data

// Counters
byte i, i2;
unsigned long int c; // Counter
unsigned long int cmax = 0xFFFFFFFF;
bool usecmax = false; 

// ===================== Hardware GUI Streaming =====================
// TBD

// Operational Core
void setup() {
  // debug
  Serial.begin(500000);
  
  // Digital Input
  for (i = 0; i < ndin; i++){
    pinMode(dins[i], INPUT);
  }

  // Digital Output
  for (i = 0; i < ndout; i++){
    pinMode(douts[i], OUTPUT);
    digitalWrite(douts[i], LOW);
  }

  // Pulse output
  pinMode(ledpin, OUTPUT);
  digitalWrite(ledpin, LOW);

  // Timers
  tnow = micros();
  tnowmillis = tnow / 1000;

  // I2c
  Wire.setSDA(0);
  Wire.setSCL(1);

  #if i2c_data
    Wire.begin();
  #endif
}

void loop() {
  // Timers
  tnow = micros();
  tnowmillis = tnow / 1000;

  if (pulse){
    if ((tnow - t1) >= ts){
      // Gets in here if cycle is up and no donotread flag
      
      // Reset timer
      t1 = tnow;
      
      // Get datain
      #if i2c_data
        if (i2c_data_use){
          // If actually use
          // Save space for data
          dnow = 0;
          requesti2c_data();
        }
        else{
          // Not using
          dnow = 1;
        }
      #else
        // Old (no i2c data) just write a 1 for aliasing check
        dnow = 1;
      #endif

      // Shift in digital data
      for (i = 0; i < ndin; i++){
        dnow = (dnow << 1) + digitalRead(dins[i]);
      }

      // LED
      ledon = !ledon;
      digitalWrite(ledpin, ledon);

      // Flag data ready
      datagood = true;
    }
  }
  
  // Get Serial data out
  if (datagood){
    c++;

    // Exceeded the predetermined number of datapoints
    if (usecmax && (c >= cmax)){
      pulse = 0;
      ledon = false;
      digitalWrite(ledpin, LOW);
      digitalWrite(douts[0], LOW);
    }

    // Use dout1 to send sync sig
    tsend = tnow;
    sync_high = true;
    digitalWrite(douts[1], HIGH);

    // Get 2 bytes of i2c data
    #if i2c_data
      if (i2c_data_use){
        geti2c_data();
        dnow = dnow + dnow2;
      }      
    #endif
      
    // Get DC
    dout = dnow;

    // Get Analog
    #if (debugmode)
      adc_get();
    #else
      adc_get_noconv();
    #endif
    
    // Send data
    #if (debugmode)
      senddata_debug();
    #else
      senddata();
    #endif
    datagood = false;
  }

  // Terminate sync
  if (sync_high && ((tnow - tsend) >= syncwidth)){
    digitalWrite(douts[1], LOW);
    sync_high = false;
  }
}
