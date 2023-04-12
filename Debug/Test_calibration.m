%% Serial list
serialportlist

%% Open
clear
picodaq_serial = serialport(sprintf('COM%i', 16), 500000);
disp('Opened.')

%% Reset ADC
write(picodaq_serial, [7 0], 'uint8');

%% EEPROM
write(picodaq_serial, [35 4], 'uint8');
pause(0.1);
echo = read(picodaq_serial, 4, 'uint8');
echo(3) * 65536 + echo(2) * 256 + echo(1)

%% Set DAQ
% 2500 Hz
write(picodaq_serial, [2 25], 'uint8');

% Analog buffer Depth = 2
write(picodaq_serial, [10 2], 'uint8');

% ADC freq = 2 (8 kSPS)
write(picodaq_serial, [3 2], 'uint8');

% Cmax
cmax = 15000;
cmax1 = floor(cmax / 256);
cmax2 = cmax - cmax1 * 256;
write(picodaq_serial, [41 cmax1], 'uint8');
write(picodaq_serial, [42 cmax2], 'uint8');
write(picodaq_serial, [43 1], 'uint8');

% EEPROM
write(picodaq_serial, [21 0], 'uint8');

% Voltage
vcal = 4096;
cmax1 = floor(vcal / 256);
cmax2 = vcal - cmax1 * 256;
write(picodaq_serial, [33 cmax1], 'uint8');
write(picodaq_serial, [34 cmax2], 'uint8');

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

%% Show ADC
write(picodaq_serial, [8 0], 'uint8');

pause(0.1);
vec = [];
ind = 0;
while picodaq_serial.NumBytesAvailable > 0
    ind = ind + 1;
    vec(ind) = read(picodaq_serial, 1, 'uint8');
end
char(vec)

%% Setup
n = 12500;
chunk = 1250;
k = zeros(n/chunk, 6 * chunk);
flush(picodaq_serial,"input")

% Start
i = 0;
disp('Start');
write(picodaq_serial, [1 0], 'uint8');
tic
while i < (n / chunk)
    fprintf('Chunk %i/%i\n', i, n/chunk);
    i = i + 1;        
    k(i,:) = read(picodaq_serial, 6 * chunk, 'int32');       
end
toc
write(picodaq_serial, [0 0], 'uint8');
disp('End');

pause(0.1)
picodaq_serial.NumBytesAvailable
flush(picodaq_serial,"input")

% Rearrange data
data = reshape(k', 6, []);
data(3:6,:) = data(3:6,:) / 2^23 * 1.2 * 8;
plot(data(3:6,:)')

%% Offset Ch0
offset = mean(data(3,:));
if offset > 0
    offset = round(offset * 2^23 / 1.2 / 8);
else
    offset = round(offset * 2^23 / 1.2 / 8 + 2^24);
end
disp(offset)

%% Offset Ch1
offset = mean(data(4,:));
if offset > 0
    offset = round(offset * 2^23 / 1.2 / 8);
else
    offset = round(offset * 2^23 / 1.2 / 8 + 2^24);
end
disp(offset)

%% Offset Ch2
offset = mean(data(5,:));
if offset > 0
    offset = round(offset * 2^23 / 1.2 / 8);
else
    offset = round(offset * 2^23 / 1.2 / 8 + 2^24);
end
disp(offset)

%% Offset Ch3
offset = mean(data(6,:));
if offset > 0
    offset = round(offset * 2^23 / 1.2 / 8);
else
    offset = round(offset * 2^23 / 1.2 / 8 + 2^24);
end
disp(offset)

%% Gain Ch0
gain = round(4.096/mean(data(3,:))/2^-23);
disp(gain)

%% Gain Ch1
gain = round(4.096/mean(data(4,:))/2^-23);
disp(gain)

%% Gain Ch2
gain = round(4.096/mean(data(5,:))/2^-23);
disp(gain)

%% Gain Ch3
gain = round(4.096/mean(data(6,:))/2^-23);
disp(gain)

%% Load everything from EEPROM
write(picodaq_serial, [39 0], 'uint8');

%% Load Offset from EERPOM
write(picodaq_serial, [36 0], 'uint8');
write(picodaq_serial, [36 1], 'uint8');
write(picodaq_serial, [36 2], 'uint8');
write(picodaq_serial, [36 3], 'uint8');

%% Transmit offset calibration via USB from picodaq
write(picodaq_serial, [30 0], 'uint8');
read(picodaq_serial, 8, 'uint32')

%% Transmit offset calibration via USB to picodaq RAM
ocal = [0, 16767433; 1, 16760110; 2, 16768087; 3, 16769142];
for i = 1 : 4
    ocalvec = typecast(uint32(ocal(i,2)), 'uint8');
    write(picodaq_serial, [45 (i-1)*10], 'uint8');
    write(picodaq_serial, [46 ocalvec(3)], 'uint8');
    write(picodaq_serial, [45 (i-1)*10+1], 'uint8');
    write(picodaq_serial, [46 ocalvec(2)], 'uint8');
    write(picodaq_serial, [46 ocalvec(1)], 'uint8');
end
write(picodaq_serial, [28 0], 'uint8');

%% Committ offset calibration to EEPROM
write(picodaq_serial, [26 2], 'uint8');

%% Transmit offset calibration via USB to picodaq RAM
gcal = [0, 8439732; 1, 8631413; 2, 8496169; 3, 8468438];
for i = 1 : 4
    gcalvec = typecast(uint32(gcal(i,2)), 'uint8');
    write(picodaq_serial, [45 (i-1)*10+2], 'uint8');
    write(picodaq_serial, [46 gcalvec(3)], 'uint8');
    write(picodaq_serial, [45 (i-1)*10+3], 'uint8');
    write(picodaq_serial, [46 gcalvec(2)], 'uint8');
    write(picodaq_serial, [46 gcalvec(1)], 'uint8');
end
write(picodaq_serial, [29 0], 'uint8');

%% Committ gain calibration to EEPROM
write(picodaq_serial, [27 2], 'uint8');

%% Close
delete(picodaq_serial);
disp('Closed.')
