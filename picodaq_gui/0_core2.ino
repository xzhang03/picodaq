// Core 1 does buttons
void setup1(){
  Wire.setSDA(4);
  Wire.setSCL(5);
  Wire.begin(i2c_address);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent); 
}

void loop1(){
  if (datastreaming != m_i2c){
    m_i2c = datastreaming;
  }
  if (chsel != n_i2c){
    n_i2c = chsel;
  }

  if (scaledata){
    scaledata = false;
    datascale(data);
    redrawdata = true;
  }
}

void requestEvent(){
  Wire.write(m_i2c);
  Wire.write(n_i2c);
}

void receiveEvent(int howMany){
  uint16_t nbytes_received;
  nbytes_received = 0;

  // Read bytes
  while(Wire.available() > 0){
    data[nbytes_received] = Wire.read();
    nbytes_received++;
  }

  // If only 2 bytes received deterine busy
  if (nbytes_received == 2){
    if (data[0] == handshake[0]){
      if (data[1] == handshake[1]){
        // Busy
        chbusy = true;
        redrawtitle = true;
        redrawdata = true;
      }
    }
  }
  else{
    // Data good
    chbusy = false;
    redrawtitle = true;
    scaledata = true;
  }

}
