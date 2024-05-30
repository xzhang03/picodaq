void drawbuttons(){
  const uint8_t spacing = 45;
  const uint8_t KEY_W = 55;
  const uint8_t KEY_H = 40;
  const byte xoffset = 4;
  
  
  for (uint8_t b = 0; b < 5; b++){
    if (b < 4){
      tft.setFreeFont(LABEL2_FONT);
    }
    else{
      tft.setFreeFont(LABEL1_FONT);
    }
    
    key[b].initButton(&tft, fx + w + KEY_W/2 + xoffset,
        fy + b * spacing, // x, y, w, h, outline, fill, text
        KEY_W, KEY_H, TFT_WHITE, keyColor[b], TFT_WHITE,
        keyLabel[b], 1);
    key[b].drawButton();
  }
}

void touch_detect(void){
  uint16_t t_x = 0, t_y = 0; // To store the touch coordinates

  // Pressed will be set true is there is a valid touch on the screen
  bool pressed = tft.getTouch(&t_x, &t_y);

//  #if debugmode
//    if (pressed){
//      Serial.print(t_x);
//      Serial.print(" ");
//      Serial.println(t_y);
//    }
//  #endif
  
  // / Check if any key coordinate boxes contain the touch coordinates
  for (uint8_t b = 0; b < 5; b++) {
    if (pressed && key[b].contains(t_x, t_y)) {
      key[b].press(true);  // tell the button it is pressed
    } else {
      key[b].press(false);  // tell the button it is NOT pressed
    }
  }

  // Check if any key has changed state
  for (uint8_t b = 0; b < 5; b++) {

    if (b < 4) tft.setFreeFont(LABEL2_FONT);
    else tft.setFreeFont(LABEL1_FONT);

    if (key[b].justReleased()) key[b].drawButton();     // draw normal

    if (key[b].justPressed()) {
      key[b].drawButton(true);  // draw invert

      #if debugmode
        Serial.print("Button Pressed: ");
        Serial.println(b);
      #endif

      // Channel 0
      if (b == 0){
        // New channel or start channel
        if (chsel != b){
          // new channel
          chsel = b;
          redrawtitle = true;
          pausedata = true;
          tswitch = t0;
         }
        if (!datastreaming){
          // Resume streaming
          datastreaming = true;
          redrawdata = true;
        }
      }

      // Channel 1
      else if (b == 1){
        // New channel or start channel
        if (chsel != b){
          // new channel
          chsel = b;
          redrawtitle = true;
          pausedata = true;
          tswitch = t0;
         }
        if (!datastreaming){
          // Resume streaming
          datastreaming = true;
          redrawdata = true;
        }
      }

      // Channel 2
      else if (b == 2){
        // New channel or start channel
        if (chsel != b){
          // new channel
          chsel = b;
          redrawtitle = true;
          pausedata = true;
          tswitch = t0;
         }
        if (!datastreaming){
          // Resume streaming
          datastreaming = true;
          redrawdata = true;
        }
      }

      // Channel 3
      else if (b == 3){
        // New channel or start channel
        if (chsel != b){
          // new channel
          chsel = b;
          redrawtitle = true;
          pausedata = true;
          tswitch = t0;
         }
        if (!datastreaming){
          // Resume streaming
          datastreaming = true;
          redrawdata = true;
        }
      }

      // STOP
      else if (b == 4){
        redrawtitle = true;
        datastreaming = false;
        redrawdata = true;
      }
      
      delay(10); // UI debouncing
    }
  }
}
