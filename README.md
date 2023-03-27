# picodaq
A raspberry pi pico inplementation of a daq acquisition board.

## Default spec
2.5k data points per second. Each data point includes six 32-bit words that are:

0. A 32-bit hardware timestamp word.
1. A 32-bit digital data word. Bit 0-15 correspond to digital inputs 0-15. Bit 16 is always 1. Bit 17-31 are 0s.
2. Four 32-bit analog data words. These are signed integers and to convert them to voltage, use the formula V = X * 1.2 * 8 / 2^23.

## Hardwares
0. Rasperry pi pico is a 133Mhz dual M0 core microcontroller that is used to synchronize data collection and send data over USB.
1. ADC: ADS131M04 is a 4-channel, 24 bit, 32 ksps non-mux delta-sigma ADC. It can convert -1.2V to 1.2V on 4 differential channels. This ADC is clocked by SiT2024B.
2. Preprocessing: THS4531ID is a fully-differential amplifier to preprocess analog inputs by dividing the voltage differential by 8, which expands the ADC range to -9.6V to 9.6V. I chose an FDA instead of a resistor ladder to downsize signals to preserve high input impedance and reduce the battery drain of the signal source.
3. 74LVC245 is an 8-channel digital transceiver that is used as a logic shifter to convert input digital signals to 3.3V logic. Two of these were used to gather 16 channel digital data.
4. 24LC256 is a 256 byte EEPROM to store DC calibration signals, in case absolute ADC accuracy is required.

## Library
ADS131M04 pico library is here: https://github.com/xzhang03/Pico-ADS131M04

## Integration
This part has been integrated into Nanosec photometry: https://github.com/xzhang03/NidaqGUI

## Footprint
![footprint](https://github.com/xzhang03/picodaq/blob/main/PCB/FDA%20v1.1/footprint.png)
