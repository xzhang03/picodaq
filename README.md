# picodaq
A raspberry pi pico implementation of a data acquisition (DAQ) board.
![Picture](https://github.com/xzhang03/picodaq/blob/main/images/Picture.png)

## Default spec
2.5k data points per second. Each data point includes six 32-bit words that are:

0. A 32-bit hardware timestamp word.
1. A 32-bit digital data word. Bit 0-15 correspond to digital inputs 0-15. Bit 16 is always 1. Bit 17-31 are 0s.
2. Four 32-bit analog data words. These are signed integers and to convert them to voltage, use the formula V = X * 1.2 * 8 / 2^23.

| Word | Meaning | Range |
| ---- | ------- | ----- |
| 0 | Timestamp | 0 - 2^32-1 (~20 days in 2.5 kHz) |
| 1 | Digital readings 0-15 | 0B1 0000 0000 0000 0000 - 0B1 1111 1111 1111 1111 (65536 - 131071) |
| 2 | A0 | -2^23 - 2^23 (-8388608 - 8388608 => -9.6V - 9.6V) | 
| 3 | A1 | -2^23 - 2^23 (-8388608 - 8388608 => -9.6V - 9.6V) | 
| 4 | A2 | -2^23 - 2^23 (-8388608 - 8388608 => -9.6V - 9.6V) | 
| 5 | A3 | -2^23 - 2^23 (-8388608 - 8388608 => -9.6V - 9.6V) | 

## Examples
### 50 Hz sine wave (sampled at 2.5 kHz)
![Sine](https://github.com/xzhang03/picodaq/blob/main/images/sine.png)

### 50 Hz triangle wave (sampled at 2.5 kHz)
![triangle](https://github.com/xzhang03/picodaq/blob/main/images/triangle.png)

### 8.93 Hz square wave (sampled at 2.5 kHz). Made with NE555.
![Square](https://github.com/xzhang03/picodaq/blob/main/images/square.png)

### ADC noise
| | PicoDAQ | NIDAQ USB6210 |
| - | ------- | ---------- |
| RMS | 0.6 mV | 2.4 mV |
| equavalent in photometry | ~0.012% dF/F | ~0.048% dF/F |

## Hardwares
0. Rasperry pi pico is a 133Mhz dual M0 core microcontroller. One core is used exclusively to babysit the ADC, and the other core is responsible for digital data collection, data synchronization, and USB communication.
1. ADC: ADS131M04 is a 4-channel, 24 bit, 32 ksps non-mux delta-sigma ADC. It can convert -1.2V to 1.2V on 4 differential channels. This ADC is clocked by SiT2024B.
2. Preprocessing: THS4531ID is a fully-differential amplifier to preprocess analog inputs by dividing the voltage differential by 8, which expands the ADC range to -9.6V to 9.6V. I chose an FDA instead of a resistor ladder to downsize signals to preserve high input impedance and reduce the battery drain of the signal source.
3. 74LVC245 is an 8-channel digital transceiver that is used as a logic shifter to convert input digital signals to 3.3V logic. Two of these were used to gather 16 channel digital data.
4. 24LC256 is a 256 byte EEPROM to store DC calibration signals, in case absolute ADC accuracy is required.

## Library
ADS131M04 pico library is here: https://github.com/xzhang03/Pico-ADS131M04

## Getting started
Use the below function in MATLAB to get started:
```Matlab
picodaq_testpanel();
```

## Serial commands
Please see [here](https://github.com/xzhang03/picodaq/blob/main/picodaq/2_sem.ino) for an up-to-date list

## Integration
This part has been integrated into Nanosec photometry: https://github.com/xzhang03/NidaqGUI

## Footprint
![footprint](https://github.com/xzhang03/picodaq/blob/main/PCB/picodaq/footprint.png)

## I2C
I2C port is provided for collecting additional data and for i2c streaming. See pinout [here](https://github.com/xzhang03/NidaqGUI/blob/master/PCBs/I2C_notes/readme.md)

## Change log
1. v1.1 Picodaq calibration
2. v1.2 Added data collection via i2c (i2c data). 2 Bytes per data point
3. v1.3 Added i2c-based hardware data streaming (picodaq gui)
