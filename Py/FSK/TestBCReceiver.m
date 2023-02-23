fs = 2e3;
dt = 1/fs;
f0 = 300; %250; %150
f1 = 650; %625; %250

lenStorage = 20*400;
rxStorage = zeros(1,lenStorage);
t1 = [0:(lenStorage-1)]*dt;
fdelta = fs/lenStorage;

inxF0 = f0/fs;

s2 = sin(2*pi*f0*t1);
s1 = ones(1,lenStorage);
faxis = ([0:(lenStorage-1)]-lenStorage/2)/lenStorage*fs;
figure(1)
%plot(faxis,fftshift(abs(fft(s2))));

% location of 
f0loc = lenStorage/2+f0/fdelta*[-1 1];
f1loc = lenStorage/2+f1/fdelta*[-1 1];
f0loc = f0loc+1; % location in matlab
f1loc = f1loc+1; % location in matlab

maskCenter =zeros(1,lenStorage);
maskCenter((lenStorage/2+1-400):(lenStorage/2+1+399)) = 1;

mask0 = [f0loc(1)-400:f0loc(1)+399 f0loc(2)-400:f0loc(2)+399];
tmpMask0 = zeros(1,lenStorage);
tmpMask0(mask0) = 1;

mask1 = [f1loc(1)-400:f1loc(1)+399 f1loc(2)-400:f1loc(2)+399];
tmpMask1 = zeros(1,lenStorage);
tmpMask1(mask1) = 1;


% generation of the frequency shifted sequence
symLengthInT = 40e-3;
symSamples = 0:dt:((symLengthInT - dt));
lenSymbol=length(symSamples);
reff0 = cos(2*pi*f0*symSamples)>0;
reff1 = cos(2*pi*f1*symSamples)>0;
reff=[reff0; reff1]*2-1;

filterMovMean= zeros(1,lenStorage);
filterMovMean(1:lenSymbol) = 1;
filterMovMeanf = conj(fftshift(fft(filterMovMean)));

mask00 = conj(fftshift(fft(reff0,lenStorage))).*tmpMask0;
mask11 = conj(fftshift(fft(reff1,lenStorage))).*tmpMask1;

% spectrum of the ereference symbol
tmpBuffer = zeros(1,lenStorage);
tmpBuffer(1:lenSymbol) = reff0;
%plot(fftshift(abs(fft(tmpBuffer))));

% generate a test vector 
tmp = zeros(1,80);
s1tmp = tmp;
for i1 =2:100
  if randn(1,1)>0
   s1tmp = [s1tmp reff0];
  else
   s1tmp = [s1tmp reff1];
  end
end

%%
% the receiver algorithm 
% filtering the frequency component
% shifting to zero frequency 
% computing delta between two shifts

% simple frequency error 
fe  = 0;
ferror = exp(j*2*pi*fe.*t1);
inputSignal = s1tmp.*ferror;
% signal 0
tmpSignal = fftshift(fft(inputSignal));
tmp00 = tmpSignal.*mask00; % masking and matchoing filtering with freq modulated signal
tmp01 = ifft(fftshift(tmp00));
tmp02 = movmean(abs(tmp01),lenSymbol);
tmp03 = ifft(fftshift((fftshift(fft(tmp01.*(exp(-j*2*pi*f0.*t1)+exp(j*2*pi*f0.*t1))))).*maskCenter));

tmp000 = s1tmp.*(exp(-j*2*pi*f0.*t1)+exp(j*2*pi*f0.*t1));
tmp001 = ifft(fftshift((fftshift(fft(tmp000))).*maskCenter));
tmp002 = movmean(abs(tmp001),lenSymbol);

% that is the working version movmean needed for doppler resistance 
tmp0000 = tmp01.*(exp(-j*2*pi*f0.*t1)+exp(j*2*pi*f0.*t1));
tmp0001 = ifft(fftshift((fftshift(fft(tmp0000))).*maskCenter));
tmp0002 = movmean(abs(tmp0001),lenSymbol/2);

%%
% shifting in frequency domain
f0shift = f0/fs*lenStorage;
g01 = zeros(1,lenStorage);
g02 = zeros(1,lenStorage);
g01((f0shift+1):end)=tmp00(1:(end-f0shift));
g02(1:(end-f0shift))=tmp00((f0shift+1):end);
g03 = ifft(fftshift((g01+g02).*maskCenter));
g04 = movmean(abs(g03),lenSymbol/2);

%% 
% signal 1
tmp10 = tmpSignal.*mask11;
tmp11 = ifft(fftshift(tmp10));
tmp12 = movmean(abs(tmp11),lenSymbol);
tmp13 = ifft(fftshift((fftshift(fft(tmp11.*(exp(-j*2*pi*f1.*t1)+exp(j*2*pi*f1.*t1))))).*maskCenter));

tmp100 = s1tmp.*(exp(-j*2*pi*f1.*t1)+exp(j*2*pi*f1.*t1));
tmp101 = ifft(fftshift((fftshift(fft(tmp100))).*maskCenter));
tmp102 = movmean(abs(tmp101),lenSymbol);

tmp1000 = tmp11.*(exp(-j*2*pi*f1.*t1)+exp(j*2*pi*f1.*t1));
tmp1001 = ifft(fftshift((fftshift(fft(tmp1000))).*maskCenter));
tmp1002 = movmean(abs(tmp1001),lenSymbol/2);

% shifting in frequency domain
f1shift = f1/fs*lenStorage;
g11 = zeros(1,lenStorage);
g12 = zeros(1,lenStorage);
g11((f1shift+1):end)=tmp10(1:(end-f1shift));
g12(1:(end-f1shift))=tmp10((f1shift+1):end);
g13 = ifft(fftshift((g11+g12).*maskCenter));
g14 = movmean(abs(g13),lenSymbol/2);


%%
% tmp0002 and g04 are the same 
% tmp1002 and g14 are the same
% decV does not work if there is Doppler error
decV = real(tmp03-tmp13);
decV2 = real(tmp0002-tmp1002);
decV3 = real(g04-g14);
decVShaped = reshape(decV2,lenSymbol,lenStorage/lenSymbol);

%plot(1:8000,real(tmp01),1:8000,real(tmp11))