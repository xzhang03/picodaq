// This is the ADC core and the non-streaming serial core
// Serial is used for setting up picoDAQ but we avoid communication during data upload

// ADC Core
void setup1() {
  // clk_pin, miso_pin, mosi_pin, cs_pin, drdy_pin, reset_pin
  adc.begin(clk_pin, miso_pin, mosi_pin, cs_pin, drdy_pin, reset_pin);

  // Input mode
  setinputsDIFF();

  // Freq
  adc.setOsr(adc_fs);

  // Gain
  for (i = 0; i < nain; i++){
    adc.setChannelPGA(i, adc_gain);
  }

  // eeprom
  #if autoeeini
    ee.begin();
  #endif
  
  // Delay
  delay(100);
      
  // Ping adc on startup
  #if pingadconstartup
    nchecksleft = nchecks;
  #endif
}

void loop1() {
  // Get Serial in
  if (Serial.available() >= 2) {
    // Read 2 bytes
    m = Serial.read();
    n = Serial.read();

    // Parse serial
    parseserial();
  }

  if (pulse) {
    // Pulsing
    if (adc.isDataReady()) { // Using non-interrupt isdataready to not mess up i2c
      // Read divide
      adc_readdiv();

      // Save
      ab0[adepthw] = anow0;
      ab1[adepthw] = anow1;
      ab2[adepthw] = anow2;
      ab3[adepthw] = anow3;

      // Move pointer
      adepthw++;
      if (adepthw >= adepth_max) {
        adepthw = 0;
      }
    }
  }
  else {
    // Non-pulsing - good to change parameters;
    #if i2cstreaming
      if (i2c_streaming_use){
        // Reading in the background if streaming is turned on
        if (adc.isDataReady()) { // Using non-interrupt isdataready to not mess up i2c
          res = adc.readADC(); // Read
        }
      }
    #endif

    // Non-pulsing - good to ping adc status
    #if pingadconstartup
      if (nchecksleft > 0){     
        uint16_t status_ping = adc.readRegister(REG_STATUS);
        #if debugmode
          Serial.print("ADC status:");
          printBits(status_ping, 15);
        #endif
        
        // Shrink
        status_ping = status_ping & 0B1111;
  
        // Use onboard LED
        if (status_ping > 0){
          // Ready
          digitalWrite(ledpin, HIGH);
          nchecksleft = 0;
        }
        else {
          digitalWrite(ledpin, LOW);
          nchecksleft--;
        }
      }
    #endif
  }

}

void adc_readdiv(void) {
  // Read adc and scale the values down in buffer so they can be added up during reading
  res = adc.readADC(); // Read
  anow0 = res.ch0 / adepth_max; // Divide
  anow1 = res.ch1 / adepth_max;
  anow2 = res.ch2 / adepth_max;
  anow3 = res.ch3 / adepth_max;
}

void adc_get(void) {
  // Get buffered adc data and sum,
  for (adepthr = 0; adepthr < adepth_max; adepthr++) {
    aout0 += ab0[adepthr];
    aout1 += ab1[adepthr];
    aout2 += ab2[adepthr];
    aout3 += ab3[adepthr];
  }

  // Convert
  af0 = adc.convert(aout0);
  af1 = adc.convert(aout1);
  af2 = adc.convert(aout2);
  af3 = adc.convert(aout3);
}

void adc_get_noconv(void) {
  // Get buffered adc data and sum,
  for (adepthr = 0; adepthr < adepth_max; adepthr++) {
    aout0 += ab0[adepthr];
    aout1 += ab1[adepthr];
    aout2 += ab2[adepthr];
    aout3 += ab3[adepthr];
  }
}

// Use differential inputs
void setinputsDIFF(void){
  for (i = 0; i < nain; i++){
    adc.setInputChannelSelection(i, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  }
  mux = 0;
}

// Use ground inputs
void setinputsGND(void){
  for (i = 0; i < nain; i++){
    adc.setInputChannelSelection(i, INPUT_CHANNEL_MUX_INPUT_SHORTED);
  }
  mux = 1;
}

// Use positive DC test inputs
void setinputsPOS(void){
  for (i = 0; i < nain; i++){
    adc.setInputChannelSelection(i, INPUT_CHANNEL_MUX_POSITIVE_DC_TEST_SIGNAL);
  }
  mux = 2;
}

// Use negative DC test inputs
void setinputsNEG(void){
  for (i = 0; i < nain; i++){
    adc.setInputChannelSelection(i, INPUT_CHANNEL_MUX_NEGATIVE_DC_TEST_SIGNAL);
  }
  mux = 3;
}

// Offset calibration 1 channel external
void offsetcal_ext(void){
  // Set LED high
  digitalWrite(douts[0], HIGH);

  // Set input right
  adc.setInputChannelSelection(chcal, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  delay(10);
  
  cal32_eo = 0;
  
  // Loop through
  for (ical = 0; ical < ncal; ical++){
    while (!adc.isDataReady()){
      // Waiting to make sure ADC is good
    }

    res = adc.readADC(); // Read

    switch (chcal){
      case 0:
        cal32_eo += res.ch0;
        break;
      case 1:
        cal32_eo += res.ch1;
        break;
      case 2:
        cal32_eo += res.ch2;
        break;
      case 3:
        cal32_eo += res.ch3;
        break;
    }
  }
  
  #if (debugmode)
    Serial.print("Total reading: ");
    printBits(cal32_eo, 31);
    Serial.print("Integer: ");
    Serial.println(cal32_eo);
    Serial.print("In volts: ");
    Serial.println(adc.convert(cal32_eo), 5);
  #endif

  // Divide by iterations
  cal32_eo >>= shiftcal;
  #if (debugmode)
    Serial.print("Divided by interations: ");
    printBits(cal32_eo, 31);
    Serial.print("Integer (32-bit frame): ");
    Serial.println(cal32_eo & 0xFFFFFF);
    Serial.print("In volts (32-bit frame): ");
    Serial.println(adc.convert(cal32_eo & 0xFFFFFF), 5);
  #endif
  
  // Convert to raw (24 bit two's complement)
  cal32_eo = adc.revtwoscom(cal32_eo);
  #if (debugmode)
    Serial.print("Converts to raw: ");
    printBits(cal32_eo, 31);
    Serial.print("Integer: ");
    Serial.println(cal32_eo);
    Serial.print("In volts: ");
    Serial.println(adc.convert(adc.twoscom(cal32_eo)), 5);
  #endif
  
  // Show
  #if (debugmode)
    Serial.print("CH offset: ");
    Serial.print(adc.convert(adc.twoscom(cal32_eo)), 5);
    Serial.print(" ");
    Serial.println(cal32_eo, HEX);
  #endif

  adc.setChannelOffsetCalibration(chcal, cal32_eo);
  
  // Write the calibration
  if (n == 1){
    byte addtemp = chcal * 4 + 4;
    writecal(addtemp, cal32_eo, true);
    #if debugmode
      Serial.print("Writing 1-channel external offset calibration to eeprom. Ch = ");
      Serial.println(chcal);
    #endif
  }

  switch (chcal){
    case 0:
      cal32_0o = cal32_eo;
      break;
    case 1:
      cal32_1o = cal32_eo;
      break;
    case 2:
      cal32_2o = cal32_eo;
      break;
    case 3:
      cal32_3o = cal32_eo;
      break;
  }
  
  digitalWrite(douts[0], LOW);
}

// Offset calibrations 4 channels
void offsetcal(void){
  // Set LED high
  digitalWrite(douts[0], HIGH);
  
  // Set input to gound
  setinputsGND();
  delay(10);

  cal32_0o = 0;
  cal32_1o = 0;
  cal32_2o = 0;
  cal32_3o = 0;
  
  // Loop through
  for (ical = 0; ical < ncal; ical++){
    while (!adc.isDataReady()){
      // Waiting to make sure ADC is good
    }

    res = adc.readADC(); // Read
    cal32_0o += res.ch0;
    cal32_1o += res.ch1;
    cal32_2o += res.ch2;
    cal32_3o += res.ch3;
  }

  // Divide by iterations
  cal32_0o >>= shiftcal;
  cal32_1o >>= shiftcal;
  cal32_2o >>= shiftcal;
  cal32_3o >>= shiftcal;
  
  // Convert to raw
  cal32_0o = adc.revtwoscom(cal32_0o);
  cal32_1o = adc.revtwoscom(cal32_1o);
  cal32_2o = adc.revtwoscom(cal32_2o);
  cal32_3o = adc.revtwoscom(cal32_3o);
  
  // Show
  #if (debugmode)
    Serial.print("CH0 offset: ");
    Serial.print(adc.convert(adc.twoscom(cal32_0o)), 5);
    Serial.print(" ");
    printBits(cal32_0o, 31);
    
    Serial.print("CH1 offset: ");
    Serial.print(adc.convert(adc.twoscom(cal32_1o)), 5);
    Serial.print(" ");
    printBits(cal32_1o, 31);
    
    Serial.print("CH2 offset: ");
    Serial.print(adc.convert(adc.twoscom(cal32_2o)), 5);
    Serial.print(" ");
    printBits(cal32_2o, 31);
    
    Serial.print("CH3 offset: ");
    Serial.print(adc.convert(adc.twoscom(cal32_3o)), 5);
    Serial.print(" ");
    printBits(cal32_3o, 31);

  #endif

  // Write the calibration
  if (n == 1){
    commitoffsetcal();
  }
  
  // Change inputs back
  setinputsDIFF();
  digitalWrite(douts[0], LOW);
}

// Gain calibrations 1 channel external
void gaincal_ext(float testv){
  // Set LED high
  digitalWrite(douts[0], HIGH);
  
  // Set input to gound
  adc.setInputChannelSelection(chcal, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
  delay(10);

  cal32_e = 0;

  // Test (0.16 volts)
  int32_t testsig = adc.revconvert(testv);
  Serial.print("Target reading: ");
  printBits(testsig, 31);
  
  // Loop through
  for (ical = 0; ical < ncal; ical++){
    while (!adc.isDataReady()){
      // Waiting to make sure ADC is good
    }

    res = adc.readADC(); // Read
    
    switch (chcal){
      case 0:
        cal32_e += (res.ch0 - testsig);
        break;
      case 1:
        cal32_e += (res.ch1 - testsig);
        break;
      case 2:
        cal32_e += (res.ch2 - testsig);
        break;
      case 3:
        cal32_e += (res.ch3 - testsig);
        break;
    }
  }
  
  // Divide by iterations
  cal32_e >>= shiftcal;


  // GCAL values 
  // Ugly floating point math but pico doesn't like int64_t for some strange reason, so integer division is a no go)
  // Pico is slow at integer division anyway :/
  uint32_t pow23 = pow(2,23);
  cal32_eu = (testsig * 1.0 / (testsig + cal32_e)) * pow23;
  
  // Show
  #if (debugmode)
    Serial.print("CH offset from test: ");
    Serial.print(adc.convert(cal32_e), 5);
    Serial.print(" ");
    Serial.println(testsig * 1.0 / (testsig + cal32_e), 5);
    Serial.print("Outcome: ");
    Serial.println(cal32_eu);    
  #endif

  adc.setChannelGainCalibration(chcal, cal32_eu);
  
  // Write the calibration
  if (n == 1){
    byte addtemp = chcal * 4 + 20;
    writecal(addtemp, cal32_eu, true);
    #if debugmode
      Serial.print("Writing 1-channel external gain calibration to eeprom. Ch = ");
      Serial.println(chcal);
    #endif
  }

  switch (chcal){
    case 0:
      cal32_0u = cal32_eu;
      break;
    case 1:
      cal32_1u = cal32_eu;
      break;
    case 2:
      cal32_2u = cal32_eu;
      break;
    case 3:
      cal32_3u = cal32_eu;
      break;
  }
  
  digitalWrite(douts[0], LOW);
}

// Gain calibrations
void gaincal(float testv){
  // Set LED high
  digitalWrite(douts[0], HIGH);
  
  // Set input to gound
  setinputsPOS();
  delay(10);

  cal32_0 = 0;
  cal32_1 = 0;
  cal32_2 = 0;
  cal32_3 = 0;

  // Test (0.16 volts)
  int32_t testsig = adc.revconvert(testv);
  Serial.print("Target reading: ");
  printBits(testsig, 31);
  
  // Loop through
  for (ical = 0; ical < ncal; ical++){
    while (!adc.isDataReady()){
      // Waiting to make sure ADC is good
    }

    res = adc.readADC(); // Read
    cal32_0 += (res.ch0 - testsig);
    cal32_1 += (res.ch1 - testsig);
    cal32_2 += (res.ch2 - testsig);
    cal32_3 += (res.ch3 - testsig);
  }
  
  // Divide by iterations
  cal32_0 >>= shiftcal;
  cal32_1 >>= shiftcal;
  cal32_2 >>= shiftcal;
  cal32_3 >>= shiftcal;

  // GCAL values 
  // Ugly floating point math but pico doesn't like int64_t for some strange reason, so integer division is a no go)
  // Pico is slow at integer division anyway :/
  uint32_t pow23 = pow(2,23);
  cal32_0u = (testsig * 1.0 / (testsig + cal32_0)) * pow23;
  cal32_1u = (testsig * 1.0 / (testsig + cal32_1)) * pow23;
  cal32_2u = (testsig * 1.0 / (testsig + cal32_2)) * pow23;
  cal32_3u = (testsig * 1.0 / (testsig + cal32_3)) * pow23;
  
  // Show
  #if (debugmode)
    Serial.print("CH0 offset from test: ");
    Serial.print(adc.convert(cal32_0), 5);
    Serial.print(" ");
    Serial.println(testsig * 1.0 / (testsig + cal32_0), 5);
    Serial.print("Outcome: ");
    Serial.println(cal32_0u);
    
    Serial.print("CH1 offset from test: ");
    Serial.print(adc.convert(cal32_1), 5);
    Serial.print(" ");
    Serial.println(testsig * 1.0 / (testsig + cal32_1), 5);
    Serial.print("Outcome: ");
    Serial.println(cal32_1u);
    
    Serial.print("CH2 offset from test: ");
    Serial.print(adc.convert(cal32_2), 5);
    Serial.print(" ");
    Serial.println(testsig * 1.0 / (testsig + cal32_2), 5);
    Serial.print("Outcome: ");
    Serial.println(cal32_2u);
    
    Serial.print("CH3 offset from test: ");
    Serial.print(adc.convert(cal32_3), 5);
    Serial.print(" ");
    Serial.println(testsig * 1.0 / (testsig + cal32_3), 5);
    Serial.print("Outcome: ");
    Serial.println(cal32_3u);
    
  #endif

  // Write the calibration
  if (n == 1){
    commitgaincal();
  }

  // Change inputs back
  setinputsDIFF();
  digitalWrite(douts[0], LOW);
}

void commitoffsetcal(void){
  adc.setChannelOffsetCalibration(0, cal32_0o);
  adc.setChannelOffsetCalibration(1, cal32_1o);
  adc.setChannelOffsetCalibration(2, cal32_2o);
  adc.setChannelOffsetCalibration(3, cal32_3o);

  #if debugmode
    Serial.println("Writing 4-channel offset calibration to eeprom.");
  #endif
}

void commitgaincal(void){
  adc.setChannelGainCalibration(0, cal32_0u);
  adc.setChannelGainCalibration(1, cal32_1u);
  adc.setChannelGainCalibration(2, cal32_2u);
  adc.setChannelGainCalibration(3, cal32_3u);

  #if debugmode
    Serial.println("Writing 4-channel gain calibration to eeprom.");
  #endif
}
