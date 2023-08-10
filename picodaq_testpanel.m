function picodaq_testpanel(com)
% Serial
if nargin < 1
    coms = serialportlist;
    if isempty(coms)
        disp('No open COM detected');
        return;
    elseif length(coms) == 1
        com = char(coms);
    else
        com = '';
    end
else
    if ~ischar(com)
        com = sprintf('COM%i',com);
    end
end

%% COM
if isempty(com)
    s = sprintf('%s ', coms);
    uiquestion = sprintf('Detected COM: %s', s); 
    com = char(coms(1));
else
    uiquestion = 'COM';
end
com_ui = inputdlg({uiquestion}, 'Serial port', [1 40], {com});

if isempty(com_ui{1})
    disp('No open COM detected');
    return;
else
    com = com_ui{1};
end

%% Tests
baudrate = 500000;
timestamps = [];
data2 = [];
indx = 1;
while true
    indx = funlist(indx);
    
    if isempty(indx)
        return;
    end
    
    switch indx
        case 1
            % Firmware version
            firmware_ver();
        case 2
            % Picodaq state dump
            state_dump();
        case 3
            % ADC register dump
            adcreg_dump();
        case 4
            % I2c scan
            disp('I2c scan:');
            i2c_scan()
        case 5
            % EEprom dump
            disp('64 bytes of calibration data:');
            eeprom_dump64()
        case 6
            % Record 4s
            record_4s();
        case 7
            % Record 2s
            plot_last_data();
        case 8
            % Dump USB buffer
            usb_flush();
    end
end

%% Anonymous functions
    % UI
    function indx = funlist(ini)
        fn = {'Firmware Version', 'Dump picodaq state', 'Dump ADC state', 'I2c scan',...
            'Dump calibration in EEProm', 'Record 4s', 'Plot last data', 'Dump USB buffer'};
        [indx, ~] = listdlg('PromptString', sprintf('Select test %s', com),...
            'SelectionMode','single', 'InitialValue', ini, 'ListString',fn);
    end
    
    % Firmware version
    function firmware_ver()
        serialin = serialinitial(com, baudrate);
        write(serialin, [38 0], 'uint8');
        pause(0.1);
        vec = [];
        ind = 0;
        while serialin.NumBytesAvailable > 0
            ind = ind + 1;
            vec(ind) = read(serialin, 1, 'uint8');
        end
        fprintf('%s', char(vec));
        delete(serialin);
    end

    % Picodaq state
    function state_dump()
        serialin = serialinitial(com, baudrate);
        write(serialin, [9 0], 'uint8');
        pause(0.1);
        vec = [];
        ind = 0;
        while serialin.NumBytesAvailable > 0
            ind = ind + 1;
            vec(ind) = read(serialin, 1, 'uint8');
        end
        fprintf('%s', char(vec));
        delete(serialin);
    end

    % ADC state
    function adcreg_dump()
        serialin = serialinitial(com, baudrate);
        write(serialin, [8 0], 'uint8');
        pause(0.1);
        vec = [];
        ind = 0;
        while serialin.NumBytesAvailable > 0
            ind = ind + 1;
            vec(ind) = read(serialin, 1, 'uint8');
        end
        fprintf('%s', char(vec));
        delete(serialin);
    end

    % I2c scan
    function i2c_scan()
        serialin = serialinitial(com, baudrate);
        write(serialin, [21 0], 'uint8');
        write(serialin, [44 0], 'uint8');
        pause(0.1);
        vec = [];
        ind = 0;
        while serialin.NumBytesAvailable > 0
            ind = ind + 1;
            vec(ind) = read(serialin, 1, 'uint8');
        end
        fprintf('%s', char(vec));
        delete(serialin);
    end

    % Calibration dump
    function eeprom_dump64()
        serialin = serialinitial(com, baudrate);
        write(serialin, [47 0], 'uint8');
        pause(0.1);
        echo = read(serialin, 64, 'uint8');
        disp(echo);
        delete(serialin);
    end

    % Record 4s
    function record_4s()
        % Serial
        serialin = serialinitial(com, baudrate);

        % 2500 Hz
        write(serialin, [2 25], 'uint8');
        
        % Analog buffer Depth = 2
        write(serialin, [10 2], 'uint8');
        
        % ADC freq = 2 (8 kSPS)
        write(serialin, [3 2], 'uint8');
        
        % Cmax
        cmax = 10000; % 2500 samples/s * 4s
        cmax1 = floor(cmax / 256);
        cmax2 = cmax - cmax1 * 256;
        write(serialin, [41 cmax1], 'uint8');
        write(serialin, [42 cmax2], 'uint8');
        write(serialin, [43 1], 'uint8');

        % Prepare matlab
        n = cmax;
        chunk = 1250; % Read in 1250 byte (0.5s chunks)
        k = zeros(n/chunk, 6 * chunk);
        
        % Record
        i = 0;
        disp('4-s recording start');
        write(serialin, [1 0], 'uint8');
        tic
        while i < (n / chunk)
            fprintf('Chunk %i/%i\n', i, n/chunk);
            i = i + 1;        
            k(i,:) = read(serialin, 6 * chunk, 'int32');       
        end
        toc
        write(serialin, [0 0], 'uint8');
        disp('End');
        
        pause(0.1)
        flush(serialin,"input")
        
        % Rearrange data
        data = reshape(k', 6, []);

        % Get timestamps out
        timestamps = data(1, :) / 2500;
        datad = uint32(data(2, :));
        dataa = data(3:end,:);
        
        % Initialize
        data2 = zeros(20, n);
        
        % Analog
        adivider = 2 ^ 23;
        for i = 1 : 4
            data2(i,:) =  dataa(i,:) / adivider * 1.2 * 8;
        end
        
        % Digital
        for i = 1 : 16
            data2(i+4,:) =  bitget(datad, 16 - i + 1);
        end
        delete(serialin);

        % Plot
        plotdata(timestamps, data2)
    end

    % Plot last data
    function plot_last_data()
        if isempty(data2)
            disp('No data recorded')
        else
            plotdata(timestamps, data2)
        end
    end
    
    % USB flush
    function usb_flush()
        serialin = serialinitial(com, baudrate);
        fprintf('Flushing %i bytes in computer USB input buffer\n', serialin.NumBytesAvailable);
        flush(serialin,"input");
        delete(serialin);
    end

    % Plot
    function plotdata(timestamps, data2)
        % Plot data
        figure('Position', [50 50 1600 600])
        
        % Plot analog
        for i = 1 : 4
            subplot(4,2,(i-1)*2+1);
            plot(timestamps, data2(i,:));
            title(sprintf('Channel %i: AI%i, RMS = %0.3fV', i, i-1, std(data2(i,:))));
            ylabel('V')
            if i == 4
                xlabel('Time (s)')
            end
        end
        
        % Plot digital
        groupn = 4;
        legendcell = cell(4,1);
        for i = 1 : 16/groupn
            % Find channels
            chs = (i-1)*4 + 4 + (1:4);

            % Plot
            subplot(4,2,i*2);
            plot(timestamps, data2(chs, :));

            for il = 1 : groupn
                legendcell{il} = sprintf('DI%i', chs(il)-5);
            end
            legend(legendcell);

            title(sprintf('Channel %i (DI%i) - Channel %i (DI%i)', chs(1), chs(1), chs(end)-4, chs(end)-5));
        end
        if i == 4
            xlabel('Time (s)')
        end
    end

end