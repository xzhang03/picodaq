void touch_calibrate(){
  // data not valid so recalibrate
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 0);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.println("Touch corners as indicated");

  tft.setTextFont(1);
  tft.println();

  tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.println("Calibration complete!");

  Serial.println("calibrate data");
  Serial.println(calData[0]);
  Serial.println(calData[1]);
  Serial.println(calData[2]);
  Serial.println(calData[3]);
  Serial.println(calData[4]);

}
