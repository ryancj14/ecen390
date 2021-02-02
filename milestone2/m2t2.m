clc, clear, close all;

Fs = 100e3;  %sampling rate
f1 = 3e3;     %signal frequency
f2 = 6e3;
A1 = 1;       %signal amplitude, This is peak amplitude not peak-to-peak
A2 = 2;
% We're going to use a time axis from -0.25 to 0.25 seconds, which is 0.5s wide
% This yields Fs*0.5s total samples on our time axis
t = linspace(-0.25, 0.25, Fs*0.5);
x = A1*sin(2*pi*f1*t) + A2*sin(2*pi*f2*t);

Y = fft(x);
L=length(Y);            %the total length of our fft
Y1=Y(1:L/2);            %we take the first half of the vector
f=linspace(0,Fs/2,L/2); %we create the frequency vector
plot(f,abs(Y1))
xlim([0 10000]);
xlabel("Frequency (Hz)")
ylabel("Magnitude (V)")
title("FFT of Synthesized Data")

figure;
x_ds = x(1:10:length(x));
t_ds = t(1:10:length(t));
Fs_ds = Fs/10;

Y = fft(x_ds);
L=length(Y);            %the total length of our fft
Y1=Y(1:L/2);            %we take the first half of the vector
f=linspace(0,Fs_ds/2,L/2); %we create the frequency vector
plot(f,abs(Y1))
xlabel("Frequency (Hz)")
ylabel("Magnitude (V)")
title("FFT of Decimated Synthesized Data")
 
figure;
load('light.mat')
plot(t,y);
xlabel("Time (s)")
ylabel("Magnitude (V)")
title("Optical Noise Data")

figure;
Y = fft(y);
L=length(Y);            %the total length of our fft
Y1=Y(1:L/2);            %we take the first half of the vector
f=linspace(0,Fs/2,L/2); %we create the frequency vector
plot(f,abs(Y1))
xlim([0 10000]);
xlabel("Frequency (Hz)")
ylabel("Magnitude (V)")
title("FFT of Optical Noise Data")

figure;
x_ds = y(1:10:length(y));
t_ds = t(1:10:length(t));
plot(t_ds,x_ds);
Fs_ds = Fs/10;
xlabel("Time (s)")
ylabel("Magnitude (V)")
title("Downsampled Optical Noise")

figure;
Y = fft(x_ds);
L=length(Y);            %the total length of our fft
Y1=Y(1:L/2);            %we take the first half of the vector
f=linspace(0,Fs_ds/2,L/2); %we create the frequency vector
plot(f,abs(Y1))
xlabel("Frequency (Hz)")
ylabel("Magnitude (V)")
title("FFT of Downsampled Optical Noise Data")


figure;
A1 = readmatrix('scope_31.csv');
t1 = A1(1:50000,1);
y1 = A1(1:50000,2);
plot(t1,y1)
xlim([-0.27 0.25])
xlabel("Time (s)")
ylabel("Magnitude (V)")
title("Collected Optical Noise Data")

figure;
Y = fft(y1);
L=length(Y);            %the total length of our fft
Y1=Y(1:L/2);            %we take the first half of the vector
f=linspace(0,Fs/2,L/2); %we create the frequency vector
plot(f,abs(Y1))
xlim([0 10000]);
xlabel("Frequency (Hz)")
ylabel("Magnitude (V)")
title("FFT of Collected Optical Noise Data")

figure;
x_ds = y1(1:10:length(y1));
t_ds = t1(1:10:length(t1));
Fs_ds = Fs/10;

Y = fft(x_ds);
L=length(Y);            %the total length of our fft
Y1=Y(1:L/2);            %we take the first half of the vector
f=linspace(0,Fs_ds/2,L/2); %we create the frequency vector
plot(f,abs(Y1))
xlabel("Frequency (Hz)")
ylabel("Magnitude (V)")
title("FFT of Downsampled Collected Optical Noise Data")

figure;
N1 = 2001;
N2 = 21;
L1 = (N1-1)/2; % the filter will go from -L to L
n1 = (-L1:L1); % this is our sample index
L2 = (N2-1)/2; % the filter will go from -L to L
n2 = (-L2:L2); % this is our sample index
f_s = 100e3;
f_corner = 5000;
h_rect_FIR = 2*f_corner/f_s*sinc(n1*2*f_corner/f_s);
plot(n1,h_rect_FIR);
xlabel("Time (s)");
ylabel("Amplitude");

figure;
hideal = transpose(h_rect_FIR);
h3 = blackmanharris(N1).*hideal; 
F = 0:f_s/1000:f_s/2;
H3 = freqz(h3,1,F,f_s); % DFT of h3
plot(F,20*log10(abs(H3)));
xlabel("Frequency (Hz)");
ylabel("Amplitude (dB)");
figure;
plot(F,abs(H3));
xlabel("Frequency (Hz)");
ylabel("Amplitude");

figure;
h_rect_FIR = 2*f_corner/f_s*sinc(n2*2*f_corner/f_s);
plot(n2,h_rect_FIR);
xlabel("Time (s)");
ylabel("Amplitude");
figure;
hideal = transpose(h_rect_FIR);
h3 = blackmanharris(N2).*hideal; 
F = 0:f_s/1000:f_s/2;
H3 = freqz(h3,1,F,f_s); % DFT of h3
plot(F,20*log10(abs(H3)));
xlabel("Frequency (Hz)");
ylabel("Amplitude (dB)");
figure;
plot(F,abs(H3));
xlabel("Frequency (Hz)");
ylabel("Amplitude");

figure;
N = 81;
L = (N-1)/2;
n = (-L:L);
h_rect_FIR = 2*f_corner/f_s*sinc(n*2*f_corner/f_s);
plot(n,h_rect_FIR);
xlabel("Time (s)");
ylabel("Amplitude");
title("N=81 sinc");
figure;
hideal = transpose(h_rect_FIR);
h3 = blackmanharris(N).*hideal; 
F = 0:f_s/1000:f_s/2;
H3 = freqz(h3,1,F,f_s); % DFT of h3
plot(F,20*log10(abs(H3)));
xlabel("Frequency (Hz)");
ylabel("Amplitude (dB)");
title("N=81 Filter");
figure;
plot(F,abs(H3));
xlabel("Frequency (Hz)");
ylabel("Amplitude");
title("N=81 Filter");

figure;
A1 = readmatrix('scope_31.csv');
t = A1(1:50000,1);
y = A1(1:50000,2);
plot(t,y)
xlim([-0.27 0.25])
xlabel("Time (s)")
ylabel("Magnitude (V)")
title("Collected Optical Noise Data")

figure;
y1 = y(1:10:length(y));
t1 = t(1:10:length(t));

h3 = blackmanharris(N);
F = 0:f_s/1000:f_s/2;
y2 = filter(h3,1,y);   %Filter the optical noise using the FIR filter

%down-sample
y3 = y2(1:10:length(y2));

%frequency spectrum
Y3 = fft(y3);
len = length(Y3);
Y3 = Y3(1:floor((len)/2));
Y3 = 2*Y3/len;

% Calculate frequency spectrum
Y1 = fft(y1);
len = length(Y1);
Y1 = Y1(1:floor((len)/2));
Y1 = 2*Y1/len;
freq = linspace(0,Fs/2,length(Y1));

%plot the 2 noise spectra
subplot(2,1,1)
plot(freq*1e-3,abs(Y1))
xlim([0 5e3])
ylim([0 .02])
xlabel("Frequency (Hz)");
ylabel("Magnitude (V)");
title("Downsampled Optical Noise");
subplot(212)
plot(freq*1e-3,abs(Y3))
xlim([0 5e3])
ylim([0 .02])
xlabel("Frequency (Hz)");
ylabel("Magnitude (V)");
title("Filtered and Downsampled Optical Noise");
