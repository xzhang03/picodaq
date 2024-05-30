// Frame of the plot
void drawframetext(void){
  
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawCentreString("0", fx, fy + h, 2);
  tft.drawCentreString("(ms)", fx + w/2, fy + h, 2);
  tft.drawCentreString("100", fx + w, fy + h, 2);

  // Ticks
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  char buf[2];
  const uint8_t textoffset = 7;

  // Text
  for (uint8_t ind = 0; ind < 5; ind++){
    // Text
    if (ind == 0){
      tft.drawRightString("V", fx, fy+yinc*ind-textoffset, 2);
    }
    else{
      sprintf(buf, "%u", 10 - ind*2);
      tft.drawRightString(buf, fx, fy+yinc*ind-textoffset, 2);
    }
  }
}

// Wipe all
void wipeall(void){  
  tft.fillRect(0, 0, 320, 240, TFT_BLACK);
}

// Wipe frame
void wipeframe(void){ 
  // Box
  tft.drawRect(fx, fy, w, h, TFT_WHITE); 
  tft.fillRect(fx + 1, fy + 1, w-2, h-2, TFT_BLACK);

  // Horizontal lines
  for (uint8_t ind = 1; ind < 5; ind++){
    // Horizontal line
    tft.drawFastHLine(fx+1, fy+yinc*ind, w-2, TFT_GREY);
  }
}


// Title
void drawtitle(void){
  const uint8_t yoffset = 2;
  const uint8_t xoffset = 30;
  char buf[10];
  tft.fillRect(fx + w/2 - xoffset, yoffset, xoffset*2, 15, TFT_BLACK);
  
  if (datastreaming){
    tft.setTextColor(TFT_VIOLET, TFT_BLACK);
    tft.setTextFont(1);
    sprintf(buf, "AI %u", chsel);
    tft.drawCentreString(buf, fx + w/2, yoffset, 2);
  }
  else if (chbusy){
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextFont(1);
    tft.drawCentreString("BUSY", fx + w/2, yoffset, 2);
  }
  else{
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.setTextFont(1);
    tft.drawCentreString("STOPPED", fx + w/2, yoffset, 2);
  }
}
