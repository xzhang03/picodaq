// Serial menu

// m table

// ========== Operational ==========
// 1: start recording
// 0: stop recording
// 2: new cycle time (10000 us /n, n in x 100 Hz)
// 9: Show all parameters
// 38: Version
// 41: Set cmax MSB (n = cmax) [Y]
// 42: shift cmax to the left by 8 bits and add n [Z]
// 43: Use cmax (n = 1 true) [[]

// ============= ADC =============
// 3: ADC freq (n = 0 - 7)
// 4: ADC gain (n = 0 - 7)
// 5: ADC offset calibration - internal (n = 1 to write ADC)
// 6: ADC gain calibration - internal (pos, n = 1 to write ADC). Not accurate.
// 7: ADC reset
// 8: ADC register read
// 10: ADC max buffer depth (n = 1 - 4) [:]
// 11: ADC differential inputs [;]
// 12: ADC ground inputs [<]
// 13: ADC positive DC test inputs [=]
// 14: ADC negative DC test inputs [>]
// 15: ADC standby [?]
// 16: ADC wakeup [@]
// 32: ADC set channel to calibrate [P]
// 33: ADC set external voltage to calibrate MSB [Q]
// 34: ADC set external voltage to calibrate LSB (will shift voltage a byte) [R]
// 17: ADC offset calibration - external single channel (n = 1 to write EEPROM) [A]
// 18: ADC gain calibration - external single channel (n = 1 to write EEPROM) [B]
// 19: Read offset cal regiser from ADC (n = channel) [C]
// 20: Read gain cal register from ADC (n = channel) [D] 
// 28: Commit offset cal t0 ADC for 4 channels from RAM to ADC [L]
// 29: Commit gain cal to ADC for 4 channels from RAM to ADC [M]
// 30: Transmit offset cal through USB for 4 channels [N]
// 31: Transmit gain cal through USB for 4 channels [O]

// ============= EEPROM =============
// 21: EEPROM initialize [E]
// 22: Check offset cal in eeprom [F]
// 23: Check gain cal in eeprom [G]
// 24: Read offset cal for 4 channels from EEPROM to RAM [H]
// 25: Read gain cal for 4 channels from EEPROM to RAM [I]
// 26: Write offset cal for 4 channels from RAM to EEPROM (n = 1 internal, 2 external) [J]
// 27: Write gain cal for 4 channels from RAM to EEPROM  (n = 1 internal, 2 external) [K]
// 35: Read 4 bytes from an eeprom address (0, 4, 8, 12<, 16@, 20D, 24H, 28L, 32P, 36T, 40X)[S]
// 36: Apply single channel offset calibration from EEPROM (n = channel) [T]
// 37: Apply single channel gain calibration from EEPROM (n = channel) [U]
// 38: Version [V]
// 39: Aplly all calibration to all channels from EEPROM [W]
// 40: Clear 4 bytes from an EEPROM address (n = address) [X]
// 44: [\]
// 45: []]
// 46: [^]
// 47: [_]
// 48: [`]
// 49: [a]
// 50: [b]
// 51: [c]
