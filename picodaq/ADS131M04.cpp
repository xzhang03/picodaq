// Raspberry pi pico library for ADS131M04
// Modified by Stephen Zhang
// Based on the work of Lucas Etchezuri

#include "Arduino.h"
#include "ADS131M04.h"
#include "SPI.h"

#define settings SPISettings(SPIfreq, MSBFIRST, SPI_MODE1)

ADS131M04::ADS131M04(){
}

uint8_t ADS131M04::writeRegister(uint8_t address, uint16_t value){
  uint16_t res;
  uint8_t addressRcv;
  uint8_t bytesRcv;
  uint16_t cmd = 0;

  digitalWrite(ADS131M04_CS_PIN, LOW);
  delayMicroseconds(1);

  cmd = (CMD_WRITE_REG) | (address << 7) | 0;

  //res = SPI.transfer16(cmd);
  SPI.transfer16(cmd);
  SPI.transfer(0x00);

  SPI.transfer16(value);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  res = SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  delayMicroseconds(1);
  digitalWrite(ADS131M04_CS_PIN, HIGH);

  addressRcv = (res & REGMASK_CMD_READ_REG_ADDRESS) >> 7;
  bytesRcv = (res & REGMASK_CMD_READ_REG_BYTES);

  if (addressRcv == address)
  {
    return bytesRcv + 1;
  }
  return 0;
}

void ADS131M04::writeRegisterMasked(uint8_t address, uint16_t value, uint16_t mask){
  // Escribe un valor en el registro, aplicando la mascara para tocar unicamente los bits necesarios.
  // No realiza el corrimiento de bits (shift), hay que pasarle ya el valor corrido a la posicion correcta

  // Leo el contenido actual del registro
  uint16_t register_contents = readRegister(address);

  // Cambio bit aa bit la mascara (queda 1 en los bits que no hay que tocar y 0 en los bits a modificar)
  // Se realiza un AND co el contenido actual del registro.  Quedan "0" en la parte a modificar
  register_contents = register_contents & ~mask;

  // se realiza un OR con el valor a cargar en el registro.  Ojo, valor debe estar en el posicion (shitf) correcta
  register_contents = register_contents | value;

  // Escribo nuevamente el registro
  writeRegister(address, register_contents);
}

bool ADS131M04::command(uint16_t cmd){
  // Issue command (6 word frame)
  digitalWrite(ADS131M04_CS_PIN, LOW);
  delayMicroseconds(1);
  SPI.transfer16(cmd);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  delayMicroseconds(1);
  digitalWrite(ADS131M04_CS_PIN, HIGH);
  return true;
  
}

uint16_t ADS131M04::readRegister(uint8_t address){
  uint16_t cmd;
  uint16_t data;

  cmd = CMD_READ_REG | (address << 7 | 0);

  digitalWrite(ADS131M04_CS_PIN, LOW);
  delayMicroseconds(1);

  //data = SPI.transfer16(cmd);
  SPI.transfer16(cmd);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  data = SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  SPI.transfer16(0x0000);
  SPI.transfer(0x00);

  delayMicroseconds(1);
  digitalWrite(ADS131M04_CS_PIN, HIGH);
  return data;
}

void ADS131M04::begin(uint8_t clk_pin, uint8_t miso_pin, uint8_t mosi_pin, uint8_t cs_pin, uint8_t drdy_pin, uint8_t reset_pin){
  // Set pins up
  ADS131M04_CS_PIN = cs_pin;
  ADS131M04_DRDY_PIN = drdy_pin;
  ADS131M04_CLK_PIN = clk_pin;
  ADS131M04_MISO_PIN = miso_pin;
  ADS131M04_MOSI_PIN = mosi_pin;
  ADS131M04_RESET_PIN = reset_pin;

  // How Pico sets SPI pins (** To be added for other boards in the future **)
  #ifdef ARDUINO_ARCH_RP204
    SPI.setRX(ADS131M04_MISO_PIN);
    SPI.setCS(ADS131M04_CS_PIN);
    SPI.setSCK(ADS131M04_CLK_PIN);
    SPI.setTX(ADS131M04_MOSI_PIN);
  #endif
  SPI.begin();

  SPI.beginTransaction(settings);
  // Configure chip select as an output
  pinMode(ADS131M04_CS_PIN, OUTPUT);
  // Configure DRDY as as input
  pinMode(ADS131M04_DRDY_PIN, INPUT);

  // Configure reset as output
  pinMode(ADS131M04_RESET_PIN, OUTPUT);
  digitalWrite(ADS131M04_RESET_PIN, HIGH);

  reset();
}

// Get data reset from STATUS register
int8_t ADS131M04::isDataReadySoft(byte channel){
  if (channel == 0)
  {
    return (readRegister(REG_STATUS) & REGMASK_STATUS_DRDY0);
  }
  else if (channel == 1)
  {
    return (readRegister(REG_STATUS) & REGMASK_STATUS_DRDY1);
  }
  else if (channel == 2)
  {
    return (readRegister(REG_STATUS) & REGMASK_STATUS_DRDY2);
  }
  else if (channel == 3)
  {
    return (readRegister(REG_STATUS) & REGMASK_STATUS_DRDY3);
  }
  else
  {
    return -1;
  }
}

bool ADS131M04::isResetStatus(void){
  return (readRegister(REG_STATUS) & REGMASK_STATUS_RESET);
}

bool ADS131M04::isLockSPI(void){
  return (readRegister(REG_STATUS) & REGMASK_STATUS_LOCK);
}

bool ADS131M04::setDrdyFormat(uint8_t drdyFormat){
  if (drdyFormat > 1)
  {
    return false;
  }
  else
  {
    writeRegisterMasked(REG_MODE, drdyFormat, REGMASK_MODE_DRDY_FMT);
    return true;
  }
}

bool ADS131M04::setDrdyStateWhenUnavailable(uint8_t drdyState){
  if (drdyState > 1)
  {
    return false;
  }
  else
  {
    writeRegisterMasked(REG_MODE, drdyState < 1, REGMASK_MODE_DRDY_HiZ);
    return true;
  }
}

bool ADS131M04::setPowerMode(uint8_t powerMode){
  if (powerMode > 3)
  {
    return false;
  }
  else
  {
    writeRegisterMasked(REG_CLOCK, powerMode, REGMASK_CLOCK_PWR);
    return true;
  }
}

bool ADS131M04::setOsr(uint16_t osr){
  if (osr > 7)
  {
    return false;
  }
  else
  {
    writeRegisterMasked(REG_CLOCK, osr << 2 , REGMASK_CLOCK_OSR);
    return true;
  }
}

bool ADS131M04::setChannelEnable(uint8_t channel, uint16_t enable){
  bool returnval = false;
  
  if (channel > 3)
  {
    returnval = false;
  }
  if (channel == 0)
  {
    writeRegisterMasked(REG_CLOCK, enable << 8, REGMASK_CLOCK_CH0_EN);
    returnval = true;
  }
  else if (channel == 1)
  {
    writeRegisterMasked(REG_CLOCK, enable << 9, REGMASK_CLOCK_CH1_EN);
    returnval = true;
  }
  else if (channel == 2)
  {
    writeRegisterMasked(REG_CLOCK, enable << 10, REGMASK_CLOCK_CH2_EN);
    returnval = true;
  }
  else if (channel == 3)
  {
    writeRegisterMasked(REG_CLOCK, enable << 11, REGMASK_CLOCK_CH3_EN);
    returnval = true;
  }
  return returnval;
}

bool ADS131M04::setChannelPGA(uint8_t channel, uint16_t pga){
  bool returnval = false;
  
  if (channel > 3)
  {
    returnval = false;
  }
  if (channel == 0)
  {
    writeRegisterMasked(REG_GAIN, pga, REGMASK_GAIN_PGAGAIN0);
    returnval = true;
  }
  else if (channel == 1)
  {
    writeRegisterMasked(REG_GAIN, pga << 4, REGMASK_GAIN_PGAGAIN1);
    returnval = true;
  }
  else if (channel == 2)
  {
    writeRegisterMasked(REG_GAIN, pga << 8, REGMASK_GAIN_PGAGAIN2);
    returnval = true;
  }
  else if (channel == 3)
  {
    writeRegisterMasked(REG_GAIN, pga << 12, REGMASK_GAIN_PGAGAIN3);
    returnval = true;
  }

  return returnval;
}

void ADS131M04::setGlobalChop(uint16_t global_chop)
{
  writeRegisterMasked(REG_CFG, global_chop << 8, REGMASK_CFG_GC_EN);
}

void ADS131M04::setGlobalChopDelay(uint16_t delay)
{
  writeRegisterMasked(REG_CFG, delay << 9, REGMASK_CFG_GC_DLY);
}

bool ADS131M04::setInputChannelSelection(uint8_t channel, uint8_t input){
  bool returnval = false;
  
  if (channel > 3)
  {
    returnval = false;
  }
  if (channel == 0)
  {
    writeRegisterMasked(REG_CH0_CFG, input, REGMASK_CHX_CFG_MUX);
    returnval = true;
  }
  else if (channel == 1)
  {
    writeRegisterMasked(REG_CH1_CFG, input, REGMASK_CHX_CFG_MUX);
    returnval = true;
  }
  else if (channel == 2)
  {
    writeRegisterMasked(REG_CH2_CFG, input, REGMASK_CHX_CFG_MUX);
    returnval = true;
  }
  else if (channel == 3)
  {
    writeRegisterMasked(REG_CH3_CFG, input, REGMASK_CHX_CFG_MUX);
    returnval = true;
  }

  return returnval;
}

bool ADS131M04::setChannelOffsetCalibration(uint8_t channel, int32_t offset){

  uint16_t MSB = offset >> 8;
  uint8_t LSB = offset;
  bool returnval = false;
  
  if (channel > 3)
  {
    returnval = false;
  }
  if (channel == 0)
  {
    writeRegisterMasked(REG_CH0_OCAL_MSB, MSB, 0xFFFF);
    writeRegisterMasked(REG_CH0_OCAL_LSB, LSB << 8, REGMASK_CHX_OCAL0_LSB);
    returnval = true;
  }
  else if (channel == 1)
  {
    writeRegisterMasked(REG_CH1_OCAL_MSB, MSB, 0xFFFF);
    writeRegisterMasked(REG_CH1_OCAL_LSB, LSB << 8, REGMASK_CHX_OCAL0_LSB);
    returnval = true;
  }
  else if (channel == 2)
  {
    writeRegisterMasked(REG_CH2_OCAL_MSB, MSB, 0xFFFF);
    writeRegisterMasked(REG_CH2_OCAL_LSB, LSB << 8, REGMASK_CHX_OCAL0_LSB);
    returnval = true;
  }
  else if (channel == 3)
  {
    writeRegisterMasked(REG_CH3_OCAL_MSB, MSB, 0xFFFF);
    writeRegisterMasked(REG_CH3_OCAL_LSB, LSB << 8 , REGMASK_CHX_OCAL0_LSB);
    returnval = true;
  }
  return returnval;
}

bool ADS131M04::setChannelGainCalibration(uint8_t channel, uint32_t gain){

  uint16_t MSB = gain >> 8;
  uint8_t LSB = gain;
  bool returnval = false;
  
  if (channel > 3)
  {
    returnval = false;
  }
  if (channel == 0)
  {
    writeRegisterMasked(REG_CH0_GCAL_MSB, MSB, 0xFFFF);
    writeRegisterMasked(REG_CH0_GCAL_LSB, LSB << 8, REGMASK_CHX_GCAL0_LSB);
    returnval = true;
  }
  else if (channel == 1)
  {
    writeRegisterMasked(REG_CH1_GCAL_MSB, MSB, 0xFFFF);
    writeRegisterMasked(REG_CH1_GCAL_LSB, LSB << 8, REGMASK_CHX_GCAL0_LSB);
    returnval = true;
  }
  else if (channel == 2)
  {
    writeRegisterMasked(REG_CH2_GCAL_MSB, MSB, 0xFFFF);
    writeRegisterMasked(REG_CH2_GCAL_LSB, LSB << 8, REGMASK_CHX_GCAL0_LSB);
    returnval = true;
  }
  else if (channel == 3)
  {
    writeRegisterMasked(REG_CH3_GCAL_MSB, MSB, 0xFFFF);
    writeRegisterMasked(REG_CH3_GCAL_LSB, LSB << 8, REGMASK_CHX_GCAL0_LSB);
    returnval = true;
  }

  return returnval; // ** "control reaches end of non-void function"
}

// non-interrrupt isDataReady implementation
bool ADS131M04::isDataReady(){
  if (digitalRead(ADS131M04_DRDY_PIN) == HIGH)
  {
    return false;
  }
  return true;
}

// Fast two's complement
int32_t ADS131M04::twoscom(int32_t datain){
  int32_t dataout;
  
  if (bitRead(datain, 23) == 1){
    dataout = datain - 16777216;
  }
  else{
    dataout = datain;
  }
  return dataout;
}

// Convert signed 32-bit values to 24-bit two's complement
int32_t ADS131M04::revtwoscom(int32_t datain){
  int32_t dataout = datain & 0xFFFFFF;;
  return dataout;
}

adcOutput ADS131M04::readADC(void){
  uint8_t x = 0;
  uint8_t x2 = 0;
  uint8_t x3 = 0;
  int32_t aux;
  adcOutput res;

  digitalWrite(ADS131M04_CS_PIN, LOW);
  delayMicroseconds(1);

  x = SPI.transfer(0x00);
  x2 = SPI.transfer(0x00);
  SPI.transfer(0x00);

  res.status = ((x << 8) | x2);

  x = SPI.transfer(0x00);
  x2 = SPI.transfer(0x00);
  x3 = SPI.transfer(0x00);

  aux = (((x << 16) | (x2 << 8) | x3) & 0x00FFFFFF);
  aux = twoscom(aux);
  res.ch0 = aux;

  x = SPI.transfer(0x00);
  x2 = SPI.transfer(0x00);
  x3 = SPI.transfer(0x00);

  aux = (((x << 16) | (x2 << 8) | x3) & 0x00FFFFFF);
  aux = twoscom(aux);
  res.ch1 = aux;

  x = SPI.transfer(0x00);
  x2 = SPI.transfer(0x00);
  x3 = SPI.transfer(0x00);

  aux = (((x << 16) | (x2 << 8) | x3) & 0x00FFFFFF);
  aux = twoscom(aux);
  res.ch2 = aux;

  x = SPI.transfer(0x00);
  x2 = SPI.transfer(0x00);
  x3 = SPI.transfer(0x00);

  aux = (((x << 16) | (x2 << 8) | x3) & 0x00FFFFFF);
  aux = twoscom(aux);
  res.ch3 = aux;

  // CRC
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.transfer(0x00);

  delayMicroseconds(1);
  digitalWrite(ADS131M04_CS_PIN, HIGH);

  return res;
}

adcOutputraw ADS131M04::readADCraw(void){
  uint8_t x = 0;
  uint8_t x2 = 0;
  uint8_t x3 = 0;
  adcOutputraw res;

  digitalWrite(ADS131M04_CS_PIN, LOW);
  delayMicroseconds(1);

  x = SPI.transfer(0x00);
  x2 = SPI.transfer(0x00);
  SPI.transfer(0x00);

  res.status = ((x << 8) | x2);

  x = SPI.transfer(0x00);
  x2 = SPI.transfer(0x00);
  x3 = SPI.transfer(0x00);

  res.ch0 = (((x << 16) | (x2 << 8) | x3) & 0x00FFFFFF);

  x = SPI.transfer(0x00);
  x2 = SPI.transfer(0x00);
  x3 = SPI.transfer(0x00);

  res.ch1 = (((x << 16) | (x2 << 8) | x3) & 0x00FFFFFF);

  x = SPI.transfer(0x00);
  x2 = SPI.transfer(0x00);
  x3 = SPI.transfer(0x00);

  res.ch2 = (((x << 16) | (x2 << 8) | x3) & 0x00FFFFFF);

  x = SPI.transfer(0x00);
  x2 = SPI.transfer(0x00);
  x3 = SPI.transfer(0x00);

  res.ch3 = (((x << 16) | (x2 << 8) | x3) & 0x00FFFFFF);

  // CRC
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.transfer(0x00);

  delayMicroseconds(1);
  digitalWrite(ADS131M04_CS_PIN, HIGH);

  return res;
}

// Hard reset through reset pin
void ADS131M04::reset(void){
  digitalWrite(ADS131M04_RESET_PIN, LOW);
  delay(10);
  digitalWrite(ADS131M04_RESET_PIN, HIGH);
  delay(10);
}

// Convert to voltage
float ADS131M04::convert(int32_t datain){
  float volt;
  volt = datain * 1.2 / 8388608; // Voltage reference  is 1.2V
  return volt;
}

// Revert convert from voltage
int32_t ADS131M04::revconvert(float datain){
  float out;
  out = datain * 8388608 / 1.2; // Voltage reference  is 1.2V
  return out;
}
