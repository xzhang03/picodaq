%% Serial list
serialportlist

%% Open
clear
picodaq_serial = serialport(sprintf('COM%i', 16), 500000);
disp('Opened.')

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

%% Setup
n = 12500;
chunk = 1250;
k = zeros(n/chunk, 6 * chunk);

%% Start
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

%% Look at channel
ch = 2;
figure
plot(data(ch+2,:));
ft2(data(ch+2,:)',2500);
title(sprintf('Mean %0.4f | std %0.4f', mean(data(ch+2,:)), std(data(ch+2,:))))
xlim([2 1250])

%% Close
delete(picodaq_serial);
disp('Closed.')