// Test panel for ADS131M04
// Modified by Stephen Zhang
// Based on the work of Lucas Etchezuri


#include <Arduino.h>
#include "ADS131M04.h"
#include <SPI.h>

ADS131M04 adc;
adcOutput res;

bool slowmode = false;
bool standby = false;

volatile bool datagood = false;

void setup()
{
  Serial.begin(115200);
  /* 
   * pico       - ADS131M04
   * sck 18     - sck14
   * miso/rx 16 - dout 15
   * mosi/tx 19 - din 16
   * cs 17      - cs 12
   * drdy 20    - drdy 13
   * Reset 21   - reset
   */
  // clk_pin, miso_pin, mosi_pin, cs_pin, drdy_pin, reset_pin
  adc.begin(18, 16, 19, 17, 20, 21);
  
  Serial.println("");

  adc.setInputChannelSelection(0, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  adc.setInputChannelSelection(1, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  adc.setInputChannelSelection(2, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  adc.setInputChannelSelection(3, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  adc.setOsr(OSR_1024);      // 32KSPS  only with 8MHZ clock

  pinMode(20, INPUT);
  attachInterrupt(digitalPinToInterrupt(20), dataRDY, FALLING);

    
  delay(100);
  printmenu();
}

void loop(){

  if (Serial.available() > 0){
    char commandCharacter = Serial.read(); //we use characters (letters) for controlling the switch-case

    //based on the command character, we decide what to do
    switch (commandCharacter){
      case 'D': // Data streaming
        {
          Serial.println("Requesting streaming data");
          unsigned long timeAnt = millis();
          unsigned long cont = 0;
        
          while (Serial.read() != 's'){
            if (datagood)
            {
              res = adc.readADC();
              cont++;
              datagood = false;
            }
            if (millis() - timeAnt > 500)
            {
              Serial.print("SPS = ");
              Serial.print(cont);
              Serial.print(" Status = ");
              Serial.print(res.status, BIN);
        
              Serial.print(" CH0 = ");
              Serial.print(adc.convert(res.ch0), 5);
        
              Serial.print(" CH1 = ");
              Serial.print(adc.convert(res.ch1), 5);
              
              Serial.print(" CH2 = ");
              Serial.print(adc.convert(res.ch2), 5);
              
              Serial.print(" CH3 = ");
              Serial.println(adc.convert(res.ch3), 5);
              
              timeAnt = millis();
              cont = 0;
            }
          }
          
        }
        break;

      case 'N': // Data streaming no interrupt
        {
          Serial.println("Requesting streaming data no interrupt");
          unsigned long timeAnt = millis();
          unsigned long cont = 0;
        
          while (Serial.read() != 's'){
            if (adc.isDataReady())
            {
              res = adc.readADC();
              cont++;
            }
            if (millis() - timeAnt > 500)
            {
              Serial.print("SPS = ");
              Serial.print(cont);
              Serial.print(" Status = ");
              Serial.print(res.status, BIN);
        
              Serial.print(" CH0 = ");
              Serial.print(adc.convert(res.ch0), 5);
        
              Serial.print(" CH1 = ");
              Serial.print(adc.convert(res.ch1), 5);
              
              Serial.print(" CH2 = ");
              Serial.print(adc.convert(res.ch2), 5);
              
              Serial.print(" CH3 = ");
              Serial.println(adc.convert(res.ch3), 5);
              
              timeAnt = millis();
              cont = 0;
            }
          }
          
        }
        break;

      case 'I':
        Serial.print("Setting inputs: A0 - diff, A1 - shorted, A2 - Pos, A3 - Neg... ");
        adc.setInputChannelSelection(0, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
        adc.setInputChannelSelection(1, INPUT_CHANNEL_MUX_INPUT_SHORTED);
        adc.setInputChannelSelection(2, INPUT_CHANNEL_MUX_POSITIVE_DC_TEST_SIGNAL);
        adc.setInputChannelSelection(3, INPUT_CHANNEL_MUX_NEGATIVE_DC_TEST_SIGNAL);
        Serial.println("Done.");
        break;
  
      case 'R': // Register
        serialreg();
        break;

      case 'T':
        Serial.print("Resetting... ");
        adc.reset();
        Serial.println("Done.");
        break;

      case 'P':
        Serial.print("SPI resetting... ");
        adc.command(CMD_RESET);
        Serial.println("Done.");
        break;

      case 'W':
        slowmode = !slowmode;
        if (slowmode){
          adc.setOsr(OSR_16384);
        }
        else{
          adc.setOsr(OSR_1024);
        }
        Serial.print("Slow mode: ");
        Serial.println(slowmode);
        break;

      case 'M':
        printmenu();
        break;

      case 'B':
        standby = !standby;
        if (standby){
          Serial.print("SPI standby... ");
          adc.command(CMD_STANDBY);
          Serial.println("Done.");
        }
        else{
          Serial.print("SPI wakeup... ");
          adc.command(CMD_WAKEUP);
          Serial.println("Done.");
        }
        break;
    }
  } 
}

void serialreg(void){
  Serial.print("ID: ");
  printBits(adc.readRegister(REG_ID));
  Serial.print("Status: ");
  printBits(adc.readRegister(REG_STATUS));
  Serial.print("Mode: ");
  printBits(adc.readRegister(REG_MODE));
  Serial.print("OSR/Enable: ");
  printBits(adc.readRegister(REG_CLOCK));
  Serial.print("PGA: ");
  printBits(adc.readRegister(REG_GAIN));
  Serial.print("Config: ");
  printBits(adc.readRegister(REG_CFG));
  Serial.print("Threshold_MSB: ");
  printBits(adc.readRegister(THRSHLD_MSB));
  Serial.print("Threshold_LSB: ");
  printBits(adc.readRegister(THRSHLD_LSB));

  Serial.println("========= CH0 =========");
  Serial.print("CH0 CFG: ");
  printBits(adc.readRegister(REG_CH0_CFG));
  Serial.print("CH0 OCAL MSB: ");
  printBits(adc.readRegister(REG_CH0_OCAL_MSB));
  Serial.print("CH0 OCAL LSB: ");
  printBits(adc.readRegister(REG_CH0_OCAL_LSB));
  Serial.print("CH0 GCAL MSB: ");
  printBits(adc.readRegister(REG_CH0_GCAL_MSB));
  Serial.print("CH0 GCAL LSB: ");
  printBits(adc.readRegister(REG_CH0_GCAL_LSB));

  
  Serial.println("========= CH1 =========");
  Serial.print("CH1 CFG: ");
  printBits(adc.readRegister(REG_CH1_CFG));
  Serial.print("CH1 OCAL MSB: ");
  printBits(adc.readRegister(REG_CH1_OCAL_MSB));
  Serial.print("CH1 OCAL LSB: ");
  printBits(adc.readRegister(REG_CH1_OCAL_LSB));
  Serial.print("CH1 GCAL MSB: ");
  printBits(adc.readRegister(REG_CH1_GCAL_MSB));
  Serial.print("CH1 GCAL LSB: ");
  printBits(adc.readRegister(REG_CH1_GCAL_LSB));
  
  Serial.println("========= CH2 =========");
  Serial.print("CH2 CFG: ");
  printBits(adc.readRegister(REG_CH2_CFG));
  Serial.print("CH2 OCAL MSB: ");
  printBits(adc.readRegister(REG_CH2_OCAL_MSB));
  Serial.print("CH2 OCAL LSB: ");
  printBits(adc.readRegister(REG_CH2_OCAL_LSB));
  Serial.print("CH2 GCAL MSB: ");
  printBits(adc.readRegister(REG_CH2_GCAL_MSB));
  Serial.print("CH2 GCAL LSB: ");
  printBits(adc.readRegister(REG_CH2_GCAL_LSB));
  
  Serial.println("========= CH3 =========");
  Serial.print("CH3 CFG: ");
  printBits(adc.readRegister(REG_CH3_CFG));
  Serial.print("CH3 OCAL MSB: ");
  printBits(adc.readRegister(REG_CH3_OCAL_MSB));
  Serial.print("CH3 OCAL LSB: ");
  printBits(adc.readRegister(REG_CH3_OCAL_LSB));
  Serial.print("CH3 GCAL MSB: ");
  printBits(adc.readRegister(REG_CH3_GCAL_MSB));
  Serial.print("CH3 GCAL LSB: ");
  printBits(adc.readRegister(REG_CH3_GCAL_LSB));
}

void dataRDY(){
  datagood = true;
}

void printBits(int b){
  for(int i = 15; i >= 0; i--)
  {
    Serial.print(bitRead(b,i));
    if(i % 4 == 0) Serial.print(" ");
  }  
  Serial.println();
}

void printmenu(void){
  Serial.println("Commands");
  Serial.println("M: Menu");
  Serial.println("D: Read 4 channel data + speed test ('s' = stop)");
  Serial.println("N: Read 4 channel data (no interrupt) + speed test ('s' = stop)");
  Serial.println("I: Set inputs to AI, 0, POS, NEG");
  Serial.println("R: Read register");
  Serial.println("T: Reset pin");
  Serial.println("P: SPI reset");
  Serial.println("W: Slow mode");
  Serial.println("B: Standby");
}
