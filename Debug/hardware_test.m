%% Open
clear
picodaq_serial = serialport(sprintf('COM%i', 16), 500000);
disp('Opened.')

%% Show parameters
write(picodaq_serial, [9 0], 'uint8');

pause(0.1);
vec = [];
ind = 0;
while picodaq_serial.NumBytesAvailable > 0
    ind = ind + 1;
    vec(ind) = read(picodaq_serial, 1, 'uint8');
end
char(vec)

%% Show firmware info
write(picodaq_serial, [38 0], 'uint8');

pause(0.1);
vec = [];
ind = 0;
while picodaq_serial.NumBytesAvailable > 0
    ind = ind + 1;
    vec(ind) = read(picodaq_serial, 1, 'uint8');
end
disp(erase(char(vec), char(10)));

%% Set DAQ
% 2500 Hz
write(picodaq_serial, [2 25], 'uint8');

% Analog buffer Depth = 3
write(picodaq_serial, [10 3], 'uint8');

% ADC freq = 0 (8 kSPS)
write(picodaq_serial, [3 0], 'uint8');


%% Reset
write(picodaq_serial, [7 0], 'uint8');

%% Set inputs
write(picodaq_serial, [13 0], 'uint8');

%% Show ADC info
write(picodaq_serial, [8 0], 'uint8');

pause(0.1);
vec = [];
ind = 0;
while picodaq_serial.NumBytesAvailable > 0
    ind = ind + 1;
    vec(ind) = read(picodaq_serial, 1, 'uint8');
end
disp(erase(char(vec), char(10)));

%% I2c
write(picodaq_serial, [21 0], 'uint8');
write(picodaq_serial, [44 0], 'uint8');

pause(0.1);
vec = [];
ind = 0;
while picodaq_serial.NumBytesAvailable > 0
    ind = ind + 1;
    vec(ind) = read(picodaq_serial, 1, 'uint8');
end
char(vec)

%% EEPROM
write(picodaq_serial, [35 0], 'uint8');
pause(0.1);
echo = read(picodaq_serial, 4, 'uint8');

%% EEPROM dump
write(picodaq_serial, [47 0], 'uint8');
pause(0.1);
echo = read(picodaq_serial, 64, 'uint8');
echo

%% Close
delete(picodaq_serial);
disp('Closed.')