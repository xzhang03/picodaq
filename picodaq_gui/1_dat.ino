void debugdatagen(uint8_t *data){
  // Straight line
  for (uint8_t ind = 0; ind < maxind; ind++){
    data[ind] = ind * 0.8;
  }
}

void debugdatagen2(uint8_t *data){
  // Straight line
  byte counter = 0;
  byte V = 0;
  
  for (uint8_t ind = 0; ind < maxind; ind++){
    data[ind] = 100 * V;

    counter++;
    if (counter == 49){
      counter = 0;
      V = 1 - V;
    }
  }
}

void datascale(uint8_t *data){
  // Basically want to multiply by 0.75, but maybe we can do it faster
  uint16_t d;
  for (uint8_t ind = 0; ind < (maxind); ind++){ 
    d = data[ind] * 3;
    data[ind] = d >> 2;
  }
}

void dataplot(uint8_t *data){
  // Height is 200 pixels for 10V
  uint8_t l;
  
  tft.drawPixel(fx, fy+h-data[0]-1, TFT_RED);
  for (uint8_t ind = 1; ind < (maxind); ind++){
//    tft.drawPixel(ind+fx, fy+h-data[ind], TFT_RED);
//    tft.drawRect(ind+fx, fy+h-data[ind]-1, 2, 2, TFT_RED);
//    tft.drawFastVLine(ind+fx, fy+h-data[ind]-1, 2, TFT_RED);
    if (data[ind] > data[ind-1]){
      // Ticking up
      l = data[ind] - data[ind-1] + 1;
      tft.drawFastVLine(ind+fx, fy+h-data[ind]-1, l, TFT_RED);
    }
    else{
      // Ticking down
      l = data[ind-1] - data[ind] + 1;
      tft.drawFastVLine(ind+fx, fy+h-data[ind-1]-1, l, TFT_RED);
    }
  }
  
}

void datanogood(void){
  // Data busy
  wipeframe();
  tft.setTextColor(TFT_RED, TFT_GREY);
  tft.setTextSize(2);

  if (chbusy){
    tft.drawCentreString("DAQ Busy", fx + w/2, fy + h/2-15, 2);
  }
  else if (!datastreaming){
    tft.drawCentreString("Data Stopped", fx + w/2, fy + h/2-15, 2);
  }
  
  tft.setTextSize(1);
}

void dataplot_debug(uint8_t *data){
  uint8_t ind;
  
  // Pixel
  td = millis();
  for (ind = 0; ind < maxind; ind++){
    tft.drawPixel(ind+fx, fy+h-data[ind], TFT_RED);
  }
  Serial.print("Pixel plot: ");
  Serial.print(millis() - td);
  Serial.println(" ms.");

  // 2x2 rectangle
  td = millis();
  for (ind = 0; ind < maxind; ind++){
    tft.drawRect(ind+fx, fy+h-data[ind]-1, 2, 2, TFT_RED);
  }
  Serial.print("2x2 plot: ");
  Serial.print(millis() - td);
  Serial.println(" ms.");

  // 2x1 v line
  td = millis();
  for (ind = 0; ind < maxind; ind++){
    tft.drawFastVLine(ind+fx, fy+h-data[ind]-1, 2, TFT_RED);
  }
  Serial.print("2x1 V plot: ");
  Serial.print(millis() - td);
  Serial.println(" ms.");

  // 2x1 h line
  td = millis();
  for (ind = 0; ind < maxind; ind++){
    tft.drawFastHLine(ind+fx, fy+h-data[ind]-1, 2, TFT_RED);
  }
  Serial.print("1x2 H plot: ");
  Serial.print(millis() - td);;
  Serial.println(" ms.");

  // Current plot function
  td = millis();
  dataplot(data);
  Serial.print("Current real plot: ");
  Serial.print(millis() - td);;
  Serial.println(" ms.");
}

void datascale_debug(uint8_t *data){
  uint8_t ind;
  
  // Floating
  td = millis();
  for (ind = 0; ind < maxind; ind++){
    data[ind] = data[ind] * 0.75;
  }
  Serial.print("Floating scaling: ");
  Serial.print(millis() - td);
  Serial.println(" ms.");

  // Current
  td = millis();
  datascale(data);
  Serial.print("Current scaling: ");
  Serial.print(millis() - td);
  Serial.println(" ms.");
}
