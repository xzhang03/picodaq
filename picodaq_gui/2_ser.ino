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
      // Force replot
      redrawtitle = true;
      redrawdata = true;
      break;
      
    case 2:
      // Plot timing
      dataplot_debug(data);
      break;
      
    case 3:
      // touch calibration
      // Calibrate the touch screen and retrieve the scaling factors
      touch_calibrate();

      // Return
      wipeall();
      wipeframe();
      drawframetext();
      drawtitle();
      drawbuttons();
      #if debugmode
        dataplot(data);
      #endif
      break;

    case 4:
      // 4: regen data (line)
      debugdatagen(data);
      redrawdata = true;
      break;

    case 5:
      // 5: regen data (square wave)
      debugdatagen2(data);
      redrawdata = true;
      break;

    case 6:
      // 6: data scaling timing
      datascale_debug(data);
      redrawdata = true;
      break;
  }
}
