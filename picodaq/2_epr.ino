// EEPROM functions go here
bool eeprom_ini(void){
  // Initialize
  #if !autoeeini
    ee.begin();
  #endif

  bool connection = ee.isConnected();
  #if debugmode
    if (!connection){
      Serial.println("ERROR: Can't find eeprom\nstopped...");
      while (1);
    }
    Serial.println("EEPROM connected");
  #endif
  return connection;
}

bool checknew(void){
  byte newsig = ee.readByte(eesigaddr);
  Serial.print("Read signalture = ");
  Serial.print(newsig, HEX);
  Serial.print(". Expected signalture = ");
  Serial.println(eesig, HEX);
  bool eenew = (newsig != eesig);
  return eenew;
}

byte checkcal(byte addr){
  // 0 - no offset, 1 - internal, 2 - external
  byte calhere = 0;

  if (ee.readByte(addr) == eeintcal){
    calhere = 1;
  }
  else if (ee.readByte(addr) == eeextcal){
    calhere = 2;
  }
  return calhere;
}

uint32_t readcal(byte addr){
  uint32_t cal;
  byte b2, b1, b0, bi;

  bi = ee.readByte(addr);
  b2 = ee.readByte(addr + 1);
  b1 = ee.readByte(addr + 2);
  b0 = ee.readByte(addr + 3);
  
  cal = (((b2 << 16) | (b1 << 8) | b0) & 0x00FFFFFF);

  #if (debugmode)
    Serial.print("EEPROM read at addr = ");
    Serial.print(addr);
    Serial.print(": ");
    Serial.print(bi, HEX);
    Serial.print(" ");
    Serial.print(b2, HEX);
    Serial.print(" ");
    Serial.print(b1, HEX);
    Serial.print(" ");
    Serial.println(b0, HEX);
  #endif
  
  return cal;
}

bool writecal(byte addr, uint32_t calv, bool extcal){
  byte b2, b1, b0;
  b0 = calv & 0xFF;
  calv >>= 8;
  b1 = calv & 0xFF;
  calv >>= 8;
  b2 = calv & 0xFF;

  ee.updateByte(addr + 3, b0);
  #if debugmode
    Serial.print("Wrote ");
    Serial.print(b0, HEX);
    Serial.print(" to ");
    Serial.println(addr + 3);
  #endif
  
  ee.updateByte(addr + 2, b1);
  #if debugmode
    Serial.print("Wrote ");
    Serial.print(b1, HEX);
    Serial.print(" to ");
    Serial.println(addr + 2);
  #endif
  
  ee.updateByte(addr + 1, b2);
  #if debugmode
    Serial.print("Wrote ");
    Serial.print(b2, HEX);
    Serial.print(" to ");
    Serial.println(addr + 1);
  #endif

  // Write internal vs external cal
  if (extcal){
    ee.updateByte(addr, eeextcal);
    #if debugmode
      Serial.print("Wrote ");
      Serial.print(eeextcal, HEX);
      Serial.print(" to ");
      Serial.println(addr);
    #endif
  }
  else{
    ee.updateByte(addr, eeintcal);
    #if debugmode
      Serial.print("Wrote ");
      Serial.print(eeintcal, HEX);
      Serial.print(" to ");
      Serial.println(addr);
    #endif
  }

  // Tag as not new
  ee.updateByte(eesigaddr, eesig);
  #if debugmode
    Serial.print("Wrote ");
    Serial.print(eesig, HEX);
    Serial.print(" to ");
    Serial.println(eesigaddr);
  #endif
  return true;
}

// Apply 1-ch offset calibration from eeprom
bool applyoffsetcal_epr(byte chtemp){
  
  uint32_t cal;
  byte b2, b1, b0, bi;

  byte addr = chtemp * 4 + 4;

  if (checkcal(addr) > 0){
    b2 = ee.readByte(addr + 1);
    b1 = ee.readByte(addr + 2);
    b0 = ee.readByte(addr + 3);
    cal = (((b2 << 16) | (b1 << 8) | b0) & 0x00FFFFFF);
    #if (debugmode)
      Serial.print("Calibration read ");
      Serial.print(cal, HEX);
      Serial.print(" at address ");
      Serial.println(addr);
    #endif
    adc.setChannelOffsetCalibration(chtemp, cal);

    switch (chtemp){
      case 0:
        cal32_0o = cal;
        break;
      case 1: 
        cal32_1o = cal;
        break;
      case 2:
        cal32_2o = cal;
        break;
      case 3:
        cal32_3o = cal;
        break;
    }
    
    #if (debugmode)
      Serial.print("Calibration applied to channel ");
      Serial.println(chtemp);
    #endif
  }
  else{
    #if (debugmode)
      Serial.print("No calibration seen at address: ");
      Serial.println(addr);
    #endif
  }
  
  return true;
}

// Apply 1-ch gain calibration from eeprom
bool applygaincal_epr(byte chtemp){
  
  uint32_t cal;
  byte b2, b1, b0, bi;

  byte addr = chtemp * 4 + 20;

  if (checkcal(addr) > 0){
    b2 = ee.readByte(addr + 1);
    b1 = ee.readByte(addr + 2);
    b0 = ee.readByte(addr + 3);
    cal = (((b2 << 16) | (b1 << 8) | b0) & 0x00FFFFFF);
    #if (debugmode)
      Serial.print("Calibration read ");
      Serial.print(cal, HEX);
      Serial.print(" at address ");
      Serial.println(addr);
    #endif
    adc.setChannelGainCalibration(chtemp, cal);

    switch (chtemp){
      case 0:
        cal32_0u = cal;
        break;
      case 1: 
        cal32_1u = cal;
        break;
      case 2:
        cal32_2u = cal;
        break;
      case 3:
        cal32_3u = cal;
        break;
    }
    
    #if (debugmode)
      Serial.print("Calibration applied to channel ");
      Serial.println(chtemp);
    #endif
  }
  else{
    #if (debugmode)
      Serial.print("No calibration seen at address: ");
      Serial.println(addr);
    #endif
  }
  
  return true;
}

void cleareeprom(byte addr){
  ee.updateByte(addr, 0xFF);
  ee.updateByte(addr + 1, 0xFF);
  ee.updateByte(addr + 2, 0xFF);
  ee.updateByte(addr + 3, 0xFF);
  #if debugmode
    Serial.print("EEPROM cleared at address = ");
    Serial.println(addr);
  #endif
}
