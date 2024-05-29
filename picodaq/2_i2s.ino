// I2c hardware GUI streaming
void i2c_streaming_main(void){
  // Advance i2c streaming large counter
  i2c_sc_large++;
  if (i2c_sc_large == 2500){
    i2c_sc_large = 0;
  }
  if (i2c_sc_large < i2c_streaming_bytes){
    // Advance i2c streaming small counter
    i2c_sc_small = i2c_sc_large;
  }

//  #if debugmode
//    Serial.print("I2c streaming indices: ");
//    Serial.print(i2c_sc_large);
//    Serial.print(" ");
//    Serial.println(i2c_sc_small);
//  #endif

  // 0-249 -> recording data
  if (i2c_sc_large < i2c_streaming_bytes){
    if (i2c_streaming_on){
      // if turned on
      int32_t aout_stream;

      // Find which data to use
      switch (i2c_streaming_ch){
        case 0:
          aout_stream = res.ch0;
          break;
        case 1:
          aout_stream = res.ch1;
          break;
        case 2:
          aout_stream = res.ch2;
          break;
        case 3:
          aout_stream = res.ch3;
          break;
      }

      // Determine if positive
      if (bitRead(aout_stream, 23) == 0){
        // Positive: write bits 15-22
        i2c_streaming_data[i2c_sc_small] = (aout_stream >> 15) & 0xFF;
      }
      else{
        // Negative: write 0
        i2c_streaming_data[i2c_sc_small] = 0;
      }

      #if debugmode
        Serial.print("Data ");
        Serial.print(i2c_sc_small);
        Serial.print(". CH ");
        Serial.print(i2c_streaming_ch);
        Serial.print(". ");
        Serial.print(aout_stream, BIN);
        Serial.print(" => ");
        Serial.println(i2c_streaming_data[i2c_sc_small] , BIN);
      #endif
    }
  }
  
  // 250 -> send data
  else if (i2c_sc_large == i2c_streaming_bytes){
    if (i2c_streaming_on){
      #if debugmode
        ti2cdebug = micros();
      #endif
      
      Wire.beginTransmission(i2c_streaming_add);
      for (uint16_t ind = 0; ind < i2c_streaming_bytes; ind++){
        Wire.write(i2c_streaming_data[ind]);
      }
      Wire.endTransmission();

      #if debugmode
        Serial.print("I2c send (us): ");
        Serial.println(micros() - ti2cdebug);
      #endif
    }
  }
  
  // 1000 request
  else if (i2c_sc_large == 1000){
    requesti2c_data(i2c_streaming_add);
  }

  // 1250 process new data
  else if (i2c_sc_large == 1250){
    if (Wire.available() > 1){
      // Expect 2 bytes of data
      m_i2cs = Wire.read();
      n_i2cs = Wire.read();

      // Turn on streaming or not
      i2c_streaming_on = (m_i2cs > 0);

      // Channel
      i2c_streaming_ch = n_i2cs;
      #if debugmode
        Serial.print("Cycle check - Streaming: ");
        Serial.print(i2c_streaming_on);
        Serial.print(" , Ch: ");
        Serial.println(i2c_streaming_ch);
      #endif
    }
    else{
      // No 2 bytes of data, halt streaming
      i2c_streaming_on = false;
      #if debugmode
        Serial.println("Didn't get 2 bytes of i2c echo - pause streaming.");
      #endif
    }
  }
}

void i2c_streaming_busy(byte address){
  Wire.beginTransmission(address);
  Wire.write(handshake[0]);
  Wire.write(handshake[1]);
  Wire.endTransmission();
}