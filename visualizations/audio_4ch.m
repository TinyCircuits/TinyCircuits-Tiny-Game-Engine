% fid = fopen("demo_audio_s16_pcm_30sec.raw");
% demo_audio_s16_bytes = fread(fid);
% demo_audio_frequncey = 22050;
% fclose(fid);

% disp("Started!");

% demo_audio_normal = [];
% for i = 1:2:rows(demo_audio_s16_bytes)
%     byte1 = demo_audio_s16_bytes(i);
%     byte2 = demo_audio_s16_bytes(i+1);
%     sample = 0;
%     sample = bitor(sample, bitshift(byte2, 8));
%     sample = bitor(sample, byte1);
%     sample = sample / (2^(16)-1);
%     demo_audio_normal(end+1) = sample;
% endfor

% disp("Done!");

% player = audioplayer(demo_audio_normal, demo_audio_frequncey);
% play(player);




% sine lookup table generation
global lookup_table_size;
lookup_table_size = 64;

lookup_table_bits = 16
lookup_table_max_fixed_point_amplitude = (((2^lookup_table_bits))/2)-1

global lookup_table_range;
lookup_table_range = 2*pi

% Scales an angle 'x' in radians to lookup table index:
% lookup_x = lookup_x / lookup_table_range;   % Scale from 0.0 ~ lookup_table_range to 0.0 ~ 1.0
% lookup_x = lookup_x * lookup_table_size;    % Scale from 0.0 ~ 1.0 to 0.0 ~ lookup_table_size
global lookup_x_scale;
lookup_x_scale = lookup_table_size / lookup_table_range;

% Only use half a period of the wave
lookup_table_step_size = lookup_table_range/lookup_table_size

% Using only half a period of the wave to make the lookup table, will be
# all positive for sine and ready to be placed in fixed-point format
global lookup_table;
lookup_table = sin(0:lookup_table_step_size:lookup_table_range) * lookup_table_max_fixed_point_amplitude;


function retval = fast_sin (x)
    global lookup_table_size;
    global lookup_table;
    global lookup_table_range;
    global lookup_x_scale;

    % Figure out where in the first quadrant the angle
    % lies. For example the 45 degree angle in the second quadrant
    % is 3pi/4, the remainder after division by pi/2 is pi/4.
    lookup_x = mod(x, lookup_table_range);      % Find remainder of full angle after divde by lookup_table_range (our table only holds values for angles from 0 to lookup_table_range)
    lookup_x = fix(lookup_x * lookup_x_scale);  % Scale from angle between 0 to lookup_table_size
    retval = lookup_table(lookup_x+1);          % Remove the +1 in C
endfunction

x = [];
y = [];

for ix = -2*pi:0.001:2*pi
    x(end+1) = ix;
    y(end+1) = fast_sin(ix);
endfor

figure(1);
plot(x, y);
xlabel("RADIANS");
ylabel("AMPLITUDE");
title(sprintf("fast sin (-2*pi ~ 2*pi): lookup table size %d, lookup table bits %d", lookup_table_size, lookup_table_bits));




% Using sine lookup table
% fs = 11025;
fs = 22050;
% fs = 44100;
dt = 1/fs;

x = 0:dt:1;

A = (0.5 + 0.5 * fast_sin(2 * pi * 200 .* x) / lookup_table_max_fixed_point_amplitude);
B = (0.5 + 0.5 * fast_sin(2 * pi * 1300 .* x) / lookup_table_max_fixed_point_amplitude);

y = A + B - (A.*B);


figure(2);
plot(x, A);
title("A");

figure(3);
plot(x, B);
title("B");

figure(4);
plot(x, y);
title("y");


player = audioplayer(y, fs);
play(player);
