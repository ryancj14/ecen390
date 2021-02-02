clc, clear, close all;

%Load in optical noise
load('light.mat') 

%we only want 200ms of data or 200e-3*100e3 = 20000 samples
t=linspace(0,.2,20000)';
y=y(1:20000);

%create the square wave signal
freq=1471;
y1=0.1*(0.5+0.5*square(2*pi*freq*t));

%Add square wave to the noise
y2=y+y1;

figure;
plot(t,y1);
title("Time-domain Plot of Player 1 Square Wave");
xlabel("Time (s)")
ylabel("Magnitude (V)")
xlim([0 0.004])
ylim([-1 1])

figure;
plot(t,y);
title("Time-domain Plot of Optical Noise");
xlabel("Time (s)")
ylabel("Magnitude (V)")
xlim([0 0.004])
ylim([-1 1])

figure;
plot(t,y2);
title("Time-domain Plot of Summed Optical Noise and Square Wave");
xlabel("Time (s)")
ylabel("Magnitude (V)")
xlim([0 0.004])
ylim([-1 1])

Fs = 100e3;
figure;
Y=fft(y2);
L=length(Y);            %the total length of our fft
Y1=Y(1:L/2);            %we take the first half of the vector
f=linspace(0,Fs/2,L/2); %we create the frequency vector
Y2=Y1/(L/2);
plot(f,abs(Y2));
xlim([0 50000]);
xlabel("Frequency (Hz)")
ylabel("Magnitude (V)")
title("FFT of Summed Optical Noise and Square Wave")

figure;
N = 81;
L = (N-1)/2;
n = (-L:L);
Fcorner = 5000;
h_rect_FIR = 2*Fcorner/Fs*sinc(n*2*Fcorner/Fs);
hideal = transpose(h_rect_FIR);
h3 = blackmanharris(N).*hideal; 
F = 0:Fs/1000:Fs/2;
H3 = freqz(h3,1,F,Fs); % DFT of h3
plot(F,abs(H3));
xlabel("Frequency (Hz)");
ylabel("Magnitude (V)");
title("N=81 Filter");

y3 = filter(h3,1,y2);
y4 = y3(1:10:length(y3));

figure;
Y=fft(y4);
L=length(Y);            %the total length of our fft
Y1=Y(1:L/2);            %we take the first half of the vector
f=linspace(0,Fs/2,L/2); %we create the frequency vector
Y2=Y1/(L/2);
plot(f,abs(Y2));
xlim([0 50000]);
xlabel("Frequency (Hz)")
ylabel("Magnitude (V)")
title("FFT of Filtered and Down-sampled Signal")

figure;
Fs = 10000;
A = importdata("a1.txt");
B = importdata("b1.txt"); 
y5 = filter(B(:,1),A(:,1),y4);

Y=fft(y5);
L=length(Y);            %the total length of our fft
Y1=Y(1:L/2);            %we take the first half of the vector
f=linspace(0,Fs/2,L/2); %we create the frequency vector
Y2=Y1/(L/2);
plot(f,abs(Y2));
xlim([0 50000]);
xlabel("Frequency (Hz)") 
ylabel("Magnitude (V)") 
title("Signal filtered by player 1 filter");

figure;
z = filter(B(:,2),A(:,2),y4);
Y=fft(z);
L=length(Y);            %the total length of our fft
Y1=Y(1:L/2);            %we take the first half of the vector
f=linspace(0,Fs/2,L/2); %we create the frequency vector
Y2=Y1/(L/2);
plot(f,abs(Y2));
xlim([0 50000]);
ylim([0 0.06]);
xlabel("Frequency (Hz)") 
ylabel("Magnitude (V)") 
title("Signal filtered by player 2 filter");

E = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
 
for i = 1:10 
    K = filter(B(:,i), A(:,i), y4); 
    Ex = sum(abs(K).^2); 
    E(i) = Ex; 
end 
figure; 
bar(E) 
title("Signal Energy through all 10 Player Filters");
xlabel('Player'); 
ylabel('Magnitude'); 
 


