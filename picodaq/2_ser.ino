// Send data
void senddata_debug(void){
  Serial.print(c);
  Serial.print(" ");
  Serial.print(dout, BIN);
  Serial.print(" ");
  Serial.print(af0, 5);
  Serial.print(" ");
  Serial.print(af1, 5);
  Serial.print(" ");
  Serial.print(af2, 5);
  Serial.print(" ");
  Serial.println(af3, 5);
  
  // Clear
  clearout();
}

// Send data
void senddata(void){
  Serial.write((byte *) &c, 4);
  Serial.write((byte *) &dout, 4);
  Serial.write((byte *) &aout0, 4);
  Serial.write((byte *) &aout1, 4);
  Serial.write((byte *) &aout2, 4);
  Serial.write((byte *) &aout3, 4);
  
  // Clear
  clearout();
}

// Serial
void parseserial(void){
  #if (debugmode)
    m = m - '0';
    n = n - '0';
    Serial.print("Inputs: ");
    Serial.print(m);
    Serial.print(" ");
    Serial.println(n);

    if (Serial.available() > 0){
      Serial.read();
    }
  #endif

  switch (m){
    case 1:
      // Just started pulsing (picodaq running)
      pulse = true;
      t0 = tnow;
      t1 = tnow;

      // Zeroing
      c = 0;
      clearout();
      
      // Initialize ADC and ADC data buffer
      while (!adc.isDataReady()){
        // Waiting to make sure ADC is good
      }
      adepthr = 0; // Reset pointers
      adepthw = 0;
      
      // Read convert
      adc_readdiv();
      
      for (i = 0; i < adepth_max; i++){
        ab0[i] = anow0;
        ab1[i] = anow1;
        ab2[i] = anow2;
        ab3[i] = anow3;
      }
      digitalWrite(douts[0], HIGH); // Use dout0 to show that ADC is good
      
      #if (debugmode)
        Serial.print("Recording (Mode ");
        Serial.print(mux);
        Serial.print(") starts at T (s) = ");
        Serial.println(tnowmillis / 1000);
      #endif
      break;  

    case 0:
      pulse = false;
      ledon = false;
      sync_high = false;
      digitalWrite(ledpin, LOW);
      digitalWrite(douts[0], LOW);
      digitalWrite(douts[1], LOW);
      #if (debugmode)
        Serial.print("Recording stops at T (s) = ");
        Serial.println(tnowmillis / 1000);
      #endif
      break;

    case 2:
      // 2: new cycle time (10000 us /n, n in x 100 Hz)
      fs = n * 100;
      ts = 10000 / n;
      #if (debugmode)
        Serial.print("New sampling rate (Hz) = ");
        Serial.println(fs);
      #endif
      break;

    case 3:
      // 3: ADC freq (n = 0 - 7)
      adc_fs = n;
      adc.setOsr(adc_fs);
      #if (debugmode)
        Serial.print("Analog rate (kSPS) = ");
        Serial.println(32 / (pow(2, adc_fs)));
      #endif
      break;
    
    case 4:
      // 4: ADC gain (n = 0 - 7)
      adc_gain = n;
      for (i = 0; i < nain; i++){
        adc.setChannelPGA(i, adc_gain);
      }
      #if (debugmode)
        Serial.print("ADC gain = ");
        Serial.println(pow(2, adc_gain));
      #endif
      break;

    case 5:
      // 5: ADC offset calculation
      offsetcal();
      break;

    case 6:
      // 6: ADC gain calculation (pos)
      gaincal(0.16);
      break;

    case 7:
      // 7: ADC wakeup
      #if (debugmode)
        Serial.print("SPI resetting... ");
      #endif
      adc.command(CMD_RESET);
      #if (debugmode)
        Serial.println("Done.");
      #endif
      break;

    case 8: // Register
      // 8: ADC register read
      serialreg();
      break;
          
    case 9:
      // List parameters
      showpara();
      break;

    case 10:
      // 10: ADC max buffer depth (n)
      adepth_max = n;
      break;

    case 11:
      // 11: ADC differential inputs
      setinputsDIFF();
      #if (debugmode)
        Serial.println("Inputs set to differential.");
      #endif
      break;

    case 12:
      // 12: ADC ground inputs
      setinputsGND();
      #if (debugmode)
        Serial.println("Inputs set to ground.");
      #endif
      break;

    case 13:
      // 13: ADC positive DC test inputs
      setinputsPOS();
      #if (debugmode)
        Serial.println("Inputs set to positive DC test voltage.");
      #endif
      break;

    case 14:
      // 14: ADC negative DC test inputs
      setinputsNEG();
      #if (debugmode)
        Serial.println("Inputs set to negative DC test voltage.");
      #endif
      break;

    case 15:
      // 15: ADC standby
      #if (debugmode)
        Serial.print("SPI standby... ");
      #endif
      adc.command(CMD_STANDBY);
      #if (debugmode)
        Serial.println("Done.");
      #endif
      break;

    case 16:
      // 16: ADC wakeup
      #if (debugmode)
        Serial.print("SPI wakeup... ");
      #endif
      adc.command(CMD_WAKEUP);
      #if (debugmode)
        Serial.println("Done.");
      #endif
      break;

    case 17:
      // 17: ADC offset calibration - external single channel [A]
      offsetcal_ext();
      break;

    case 18:
      // 18: ADC gain calibration - external single channel (n = 1 to write) [B]
      gaincal_ext(extvfloat);
      break;

    case 19:
      // 19: Read offset cal regiser from ADC (n = channel) [C]
      switch (n){
        case 0:
          cal32_0o = adc.readRegister(REG_CH0_OCAL_MSB);
          cal32_0o = (cal32_0o << 16) + adc.readRegister(REG_CH0_OCAL_LSB);
          cal32_0o >>= 8;
          #if debugmode
            Serial.print("Calibration values read: ");
            Serial.println(cal32_0o);
          #endif
          break;
        case 1:
          cal32_1o = adc.readRegister(REG_CH1_OCAL_MSB);
          cal32_1o = (cal32_1o << 16) + adc.readRegister(REG_CH1_OCAL_LSB);
          cal32_1o >>= 8;
          #if debugmode
            Serial.print("Calibration values read: ");
            Serial.println(cal32_1o);
          #endif
          break;
        case 2:
          cal32_2o = adc.readRegister(REG_CH2_OCAL_MSB);
          cal32_2o = (cal32_2o << 16) + adc.readRegister(REG_CH2_OCAL_LSB);
          cal32_2o >>= 8;
          #if debugmode
            Serial.print("Calibration values read: ");
            Serial.println(cal32_2o);
          #endif
          break;
        case 3:
          cal32_3o = adc.readRegister(REG_CH3_OCAL_MSB);
          cal32_3o = (cal32_3o << 16) + adc.readRegister(REG_CH3_OCAL_LSB);
          cal32_3o >>= 8;
          #if debugmode
            Serial.print("Calibration values read: ");
            Serial.println(cal32_3o);
          #endif
          break;
      }
      break;

    case 20:
      // 20: Read gain cal register from ADC (n = channel) [D] 
      switch (n){
        case 0:
          cal32_0u = adc.readRegister(REG_CH0_GCAL_MSB);
          cal32_0u = (cal32_0u << 16) + adc.readRegister(REG_CH0_GCAL_LSB);
          cal32_0u >>= 8;
          #if debugmode
            Serial.print("Calibration values read: ");
            Serial.println(cal32_0u);
          #endif
          break;
        case 1:
          cal32_1u = adc.readRegister(REG_CH1_GCAL_MSB);
          cal32_1u = (cal32_1u << 16) + adc.readRegister(REG_CH1_GCAL_LSB);
          cal32_1u >>= 8;
          #if debugmode
            Serial.print("Calibration values read: ");
            Serial.println(cal32_1u);
          #endif
          break;
        case 2:
          cal32_2u = adc.readRegister(REG_CH2_GCAL_MSB);
          cal32_2u = (cal32_2u << 16) + adc.readRegister(REG_CH2_GCAL_LSB);
          cal32_2u >>= 8;
          #if debugmode
            Serial.print("Calibration values read: ");
            Serial.println(cal32_2u);
          #endif
          break;
        case 3:
          cal32_3u = adc.readRegister(REG_CH3_GCAL_MSB);
          cal32_3u = (cal32_3u << 16) + adc.readRegister(REG_CH3_GCAL_LSB);
          cal32_3u >>= 8;
          #if debugmode
            Serial.print("Calibration values read: ");
            Serial.println(cal32_3u);
          #endif
          break;
      }
      break;

    case 21:
      // 21: EEPROM initialize [E]
      eeprom_ini();
      if (checknew()){
        #if debugmode
          Serial.println("EEPROM is new.");
        #endif
      }
      else{
        #if debugmode
          Serial.println("EEPROM was written.");
        #endif
      }
      break;

    case 22:
      // 22: Check offset cal [F]
      {
        byte offsethere = checkcal(4);
        #if debugmode
          Serial.print("Offset cal in eeprom (0 = no, 1 = int, 2 = ext): ");
          Serial.println(offsethere);
        #endif
      }
      break;

    case 23:
      // 23: Check gain cal [G]
      {
        byte gainhere = checkcal(20);
        #if debugmode
          Serial.print("Gain cal in eeprom (0 = no, 1 = int, 2 = ext): ");
          Serial.println(gainhere);
        #endif
      }
      break;

    case 24:
      // 24: Read offset cal for 4 channels [H]
      cal32_0o = readcal(4);
      cal32_1o = readcal(8);
      cal32_2o = readcal(12);
      cal32_3o = readcal(16);
      #if debugmode
        Serial.println("FROM EEPROM:");
        Serial.print("CH0 offset: ");
        Serial.print(adc.convert(adc.revtwoscom(cal32_0o)), 5);
        Serial.print(" ");
        printBits(cal32_0o, 31);
        
        Serial.print("CH1 offset: ");
        Serial.print(adc.convert(adc.revtwoscom(cal32_1o)), 5);
        Serial.print(" ");
        printBits(cal32_1o, 31);
        
        Serial.print("CH2 offset: ");
        Serial.print(adc.convert(adc.revtwoscom(cal32_2o)), 5);
        Serial.print(" ");
        printBits(cal32_2o, 31);
        
        Serial.print("CH3 offset: ");
        Serial.print(adc.convert(adc.revtwoscom(cal32_3o)), 5);
        Serial.print(" ");
        printBits(cal32_3o, 31);
      #endif
      break;

    case 25:
      // 25: Read gain cal for 4 channels [I]
      cal32_0u = readcal(20);
      cal32_1u = readcal(24);
      cal32_2u = readcal(28);
      cal32_3u = readcal(32);
     
      #if debugmode
        Serial.print("CH0 offset from test: ");
        Serial.println(cal32_0u);
        
        Serial.print("CH1 offset from test: ");
        Serial.println(cal32_1u);
        
        Serial.print("CH2 offset from test: ");
        Serial.println(cal32_2u);
        
        Serial.print("CH3 offset from test: ");
        Serial.println(cal32_3u);
      #endif
      break;

    case 26:
      // 26: Write offset cal for 4 channels (n = 1 internal, 2 external) [J]
      {
        bool ext = n > 1;
        writecal(4, cal32_0o, ext);
        writecal(8, cal32_1o, ext);
        writecal(12, cal32_2o, ext);
        writecal(16, cal32_3o, ext);
        #if debugmode
          Serial.println("Offset cal written to eeprom.");
        #endif
      }
      break;

    case 27:
      // 27: Write gain cal for 4 channels (n = 1 internal, 2 external) [K]
      {
        bool ext = n > 1;
        writecal(20, cal32_0u, ext);
        writecal(24, cal32_1u, ext);
        writecal(28, cal32_2u, ext);
        writecal(32, cal32_3u, ext);
        #if debugmode
          Serial.println("Gain cal written to eeprom.");
        #endif
      }
      break;

    case 28:
      // 28: Commit offset cal t0 ADC for 4 channels [L]
      commitoffsetcal();
      #if debugmode
        Serial.println("Offset cal committed to ADC.");
      #endif
      break;

    case 29:
      // 29: Commit gain cal to ADC for 4 channels [M]
      commitgaincal();
      #if debugmode
        Serial.println("Gain cal committed to ADC.");
      #endif
      break;

    case 30: 
      // 30: Transmit offset cal through USB for 4 channels [N]
      #if debugmode
        Serial.print("CH0 offset: ");
        Serial.println(cal32_0o, BIN);
        
        Serial.print("CH1 offset: ");
        Serial.println(cal32_1o, BIN);
        
        Serial.print("CH2 offset: ");
        Serial.println(cal32_2o, BIN);
        
        Serial.print("CH3 offset: ");
        Serial.println(cal32_3o, BIN);
      #else
        {
          uint32_t chbuf;
          chbuf = 0;
          Serial.write((byte*) &chbuf, 4);
          Serial.write((byte *) &cal32_0o, 4);

          chbuf = 1;
          Serial.write((byte*) &chbuf, 4);
          Serial.write((byte *) &cal32_1o, 4);

          chbuf = 2;
          Serial.write((byte*) &chbuf, 4);
          Serial.write((byte *) &cal32_2o, 4);

          chbuf = 3;
          Serial.write((byte*) &chbuf, 4);
          Serial.write((byte *) &cal32_3o, 4);
        }
      #endif
      break;

    case 31: 
      // 31: Transmit gain cal through USB for 4 channels [O]
      #if debugmode
        Serial.print("CH0 gain: ");
        Serial.println(cal32_0u, BIN);
        
        Serial.print("CH1 gain: ");
        Serial.println(cal32_1u, BIN);
        
        Serial.print("CH2 gain: ");
        Serial.println(cal32_2u, BIN);
        
        Serial.print("CH3 gain: ");
        Serial.println(cal32_3u, BIN);
      #else
        {
          uint32_t chbuf;
          chbuf = 0;
          Serial.write((byte*) &chbuf, 4);
          Serial.write((byte *) &cal32_0u, 4);

          chbuf = 1;
          Serial.write((byte*) &chbuf, 4);
          Serial.write((byte *) &cal32_1u, 4);

          chbuf = 2;
          Serial.write((byte*) &chbuf, 4);
          Serial.write((byte *) &cal32_2u, 4);

          chbuf = 3;
          Serial.write((byte*) &chbuf, 4);
          Serial.write((byte *) &cal32_3u, 4);
        }
      #endif
      break;

    case 32:
      // 32: ADC set channel to calibrate [P]
      chcal = n;
      #if debugmode
        Serial.print("Channel to be externally calibrated: ");
        Serial.println(chcal);
      #endif
      break;

    case 33:
      // 33: ADC set external voltage to calibrate MSB [Q]
      extv = n;
      extvfloat = extv * 1.0 / 1000;
      #if debugmode
        Serial.print("External reference voltage set to (V): ");
        Serial.println(extvfloat, 5);
      #endif
      break;

    case 34:
      // 34: ADC set external voltage to calibrate LSB (will shift voltage a byte) [R]
      extv <<= 8;
      extv += n;
      extvfloat = extv * 1.0 / 1000;
      #if debugmode
        Serial.print("External reference voltage set to (V): ");
        Serial.println(extvfloat, 5);
      #endif
      break;

    case 35:
      // 35: Read 4 bytes from an eeprom address (0, 4, 8, 12<, 16@, 20D, 24H, 28L, 32P, 36T, 40X)[S]
      {
        uint32_t readval = readcal(n);
        byte bvec[4];
        bvec[3] = (readval >> 24) & 0xFF;
        bvec[2] = (readval >> 16) & 0xFF;
        bvec[1] = (readval >> 8) & 0xFF;
        bvec[0] = (readval) & 0xFF;
        Serial.write(bvec, 4);
      }
      break;

    case 36:
      // 36: Apply single channel offset calibration from EEPROM (n = channel) [T]
      applyoffsetcal_epr(n);
      break;

    case 37:
      // 37: Apply single channel gain calibration from EEPROM (n = channel) [U]
      applygaincal_epr(n);
      break;
    
    case 38:
      // 38: Version [V]
      Serial.print("Picodaq version: ");
      Serial.println(pdvers);
      Serial.print("Compiled on: ");
      Serial.print(__DATE__);
      Serial.print(" ");
      Serial.println(__TIME__);
      break;

    case 39:
      // 39: Aplly all calibration to all channels from EEPROM [W]
      {
        for (byte itemp = 0; itemp < 4; itemp++){
          applyoffsetcal_epr(itemp);
          applygaincal_epr(itemp);
        }
      }
      break;

    case 40:
      // 40: Clear 4 bytes from an EEPROM address (n = address) [X]
      cleareeprom(n);
      break;

    case 41:
      // 41: Set cmax MSB (n = cmax) [Y]
      cmax = n;
      break;

    case 42:
      // 42: shift cmax to the left by 8 bits and add n [Z]
      cmax = (cmax << 8) + n;
      break;

    case 43:
      // 43: Use cmax (n = 1 true) [[]
      usecmax = (n == 1);
      break;

    case 44:
      // 44: I2c scan [\]
      i2c_scan();
      break;

    case 45:
      // 45: Get ready to receive a byte of calibration through USB (n = cal selector) []]
      // First digit: channel
      // Second digit: 0 - first offset byte, 1 - shift offset byte, 2 - first gain byte, 3 - shift gain byte
      selcal = n;
      break;

    case 46:
      // 46: Receive a byte of calibration through USB (n = cal byte) [^]
      {
        byte chselcal = selcal / 10;
        byte typeselcal = selcal - chselcal * 10;
        switch (chselcal){
          case 0:
            switch (typeselcal){
              case 0:
                cal32_0o = n;
                break;
              case 1:
                cal32_0o = (cal32_0o << 8) + n;
                break;
              case 2:
                cal32_0u = n;
                break;
              case 3:
                cal32_0u = (cal32_0u << 8) + n;
                break;
            }
            break;
          case 1:
            switch (typeselcal){
              case 0:
                cal32_1o = n;
                break;
              case 1:
                cal32_1o = (cal32_1o << 8) + n;
                break;
              case 2:
                cal32_1u = n;
                break;
              case 3:
                cal32_1u = (cal32_1u << 8) + n;
                break;
            }
            break;
          case 2:
            switch (typeselcal){
              case 0:
                cal32_2o = n;
                break;
              case 1:
                cal32_2o = (cal32_2o << 8) + n;
                break;
              case 2:
                cal32_2u = n;
                break;
              case 3:
                cal32_2u = (cal32_2u << 8) + n;
                break;
            }
            break;
          case 3:
            switch (typeselcal){
              case 0:
                cal32_3o = n;
                break;
              case 1:
                cal32_3o = (cal32_3o << 8) + n;
                break;
              case 2:
                cal32_3u = n;
                break;
              case 3:
                cal32_3u = (cal32_3u << 8) + n;
                break;
            }
            break;
        }
      }
      break;

    case 47:
      // 47: Dump 64 bytes from eeprom [_]
      {
        byte eemprom_dump[64];
        for (byte edumpi = 0; edumpi < 64; edumpi++){
          eemprom_dump[edumpi] = ee.readByte(edumpi);
        }
        Serial.write(eemprom_dump, 64);
      }
      break;
  }
}

void showpara(void){
  Serial.println("============== Sampling ==============");
  Serial.print("Sampling rate (Hz): ");
  Serial.println(fs);
  Serial.print("Sampling interval (us): ");
  Serial.println(ts);
  Serial.print("cmax: ");
  Serial.println(cmax);
  Serial.print("usecmax: ");
  Serial.println(usecmax);
  
  Serial.println("============== Digital ==============");
  Serial.print("N digital inputs: ");
  Serial.println(ndin);
  for (i = 0; i < ndin; i++){
    Serial.print(dins[i]);
    Serial.print(" ");
  }
  Serial.println();

  Serial.println("============== Analog ==============");
  Serial.print("N analog inputs: ");
  Serial.println(nain);
  Serial.print("MUX (0 DIF, 1 GND, 2 POS, 3 NEG): ");
  Serial.println(mux);
  Serial.print("ADC sampling rate (kSPS): ");
  Serial.println(32 / (pow(2, adc_fs)));
  Serial.print("ADC gain: ");
  Serial.println(pow(2, adc_gain));
  Serial.print("ADC buffer depth: ");
  Serial.println(adepth_max);

  Serial.println("============== Calibration ==============");
  Serial.print("Channel to be externally calibrated: ");
  Serial.println(chcal);
  Serial.print("Calibration Selector (USB calibration only): ");
  Serial.println(selcal);
  Serial.print("Default external calibration voltage (mV): ");
  Serial.println(extv);
  Serial.print("Default external calibration voltage (V): ");
  Serial.println(extvfloat, 5);
  Serial.print("Ch0 Offset: ");
  Serial.println(cal32_0o);
  Serial.print("Ch0 Gain: ");
  Serial.println(cal32_0u);
  Serial.print("Ch1 Offset: ");
  Serial.println(cal32_1o);
  Serial.print("Ch1 Gain: ");
  Serial.println(cal32_1u);
  Serial.print("Ch2 Offset: ");
  Serial.println(cal32_2o);
  Serial.print("Ch2 Gain: ");
  Serial.println(cal32_2u);
  Serial.print("Ch3 Offset: ");
  Serial.println(cal32_3o);
  Serial.print("Ch3 Gain: ");
  Serial.println(cal32_3u);
}

// Clear outputs
void clearout(void){
  dout = 0;
  aout0 = 0;
  aout1 = 0;
  aout2 = 0;
  aout3 = 0;
}

// ADC registers
void serialreg(void){
  Serial.print("ID: ");
  printBits(adc.readRegister(REG_ID), 15);
  Serial.print("Status: ");
  printBits(adc.readRegister(REG_STATUS), 15);
  Serial.print("Mode: ");
  printBits(adc.readRegister(REG_MODE), 15);
  Serial.print("OSR/Enable: ");
  printBits(adc.readRegister(REG_CLOCK), 15);
  Serial.print("PGA: ");
  printBits(adc.readRegister(REG_GAIN), 15);
  Serial.print("Config: ");
  printBits(adc.readRegister(REG_CFG), 15);
  Serial.print("Threshold_MSB: ");
  printBits(adc.readRegister(THRSHLD_MSB), 15);
  Serial.print("Threshold_LSB: ");
  printBits(adc.readRegister(THRSHLD_LSB), 15);

  Serial.println("========= CH0 =========");
  Serial.print("CH0 CFG: ");
  printBits(adc.readRegister(REG_CH0_CFG), 15);
  Serial.print("CH0 OCAL MSB: ");
  printBits(adc.readRegister(REG_CH0_OCAL_MSB), 15);
  Serial.print("CH0 OCAL LSB: ");
  printBits(adc.readRegister(REG_CH0_OCAL_LSB), 15);
  Serial.print("CH0 GCAL MSB: ");
  printBits(adc.readRegister(REG_CH0_GCAL_MSB), 15);
  Serial.print("CH0 GCAL LSB: ");
  printBits(adc.readRegister(REG_CH0_GCAL_LSB), 15);

  
  Serial.println("========= CH1 =========");
  Serial.print("CH1 CFG: ");
  printBits(adc.readRegister(REG_CH1_CFG), 15);
  Serial.print("CH1 OCAL MSB: ");
  printBits(adc.readRegister(REG_CH1_OCAL_MSB), 15);
  Serial.print("CH1 OCAL LSB: ");
  printBits(adc.readRegister(REG_CH1_OCAL_LSB), 15);
  Serial.print("CH1 GCAL MSB: ");
  printBits(adc.readRegister(REG_CH1_GCAL_MSB), 15);
  Serial.print("CH1 GCAL LSB: ");
  printBits(adc.readRegister(REG_CH1_GCAL_LSB), 15);
  
  Serial.println("========= CH2 =========");
  Serial.print("CH2 CFG: ");
  printBits(adc.readRegister(REG_CH2_CFG), 15);
  Serial.print("CH2 OCAL MSB: ");
  printBits(adc.readRegister(REG_CH2_OCAL_MSB), 15);
  Serial.print("CH2 OCAL LSB: ");
  printBits(adc.readRegister(REG_CH2_OCAL_LSB), 15);
  Serial.print("CH2 GCAL MSB: ");
  printBits(adc.readRegister(REG_CH2_GCAL_MSB), 15);
  Serial.print("CH2 GCAL LSB: ");
  printBits(adc.readRegister(REG_CH2_GCAL_LSB), 15);
  
  Serial.println("========= CH3 =========");
  Serial.print("CH3 CFG: ");
  printBits(adc.readRegister(REG_CH3_CFG), 15);
  Serial.print("CH3 OCAL MSB: ");
  printBits(adc.readRegister(REG_CH3_OCAL_MSB), 15);
  Serial.print("CH3 OCAL LSB: ");
  printBits(adc.readRegister(REG_CH3_OCAL_LSB), 15);
  Serial.print("CH3 GCAL MSB: ");
  printBits(adc.readRegister(REG_CH3_GCAL_MSB), 15);
  Serial.print("CH3 GCAL LSB: ");
  printBits(adc.readRegister(REG_CH3_GCAL_LSB), 15);
}

void printBits(int32_t b, byte n){
  for(int8_t i2 = n; i2 >= 0; i2--)
  {
    Serial.print(bitRead(b,i2));
    if(i2 % 4 == 0) Serial.print(" ");
  }  
  Serial.println();
}
