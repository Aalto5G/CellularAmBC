# #!/usr/bin/python3    

import matplotlib.pyplot as plt
import random as rnd
import numpy as np

class fskReceiver():

  def __init__(self,f0,f1,fs,lenStorage,symLengthInT):  
   
   self.f0 = f0
   self.f1 = f1
   self.fs = fs
   self.lenStorage = lenStorage
   
   fdelta = fs/lenStorage;

   self.f0shift = f0/fs*lenStorage;
   self.f1shift = f1/fs*lenStorage;
   #f0loc = lenStorage/2+f0/fdelta*[-1 1];
   #f1loc = lenStorage/2+f1/fdelta*[-1 1];


   flen = 100/fs*lenStorage; # masking bandwidth

   self.maskCenter = np.zeros(lenStorage); # final filter after matching
   self.maskCenter[int(lenStorage/2-flen):int(lenStorage/2+flen-1)] = 1;

   # f0 filter in frequency domain
   #mask0 = [f0loc(1)-400:f0loc(1)+399 f0loc(2)-400:f0loc(2)+399];
   tmpMask0 = np.zeros(lenStorage);
   tmpMask0[int(self.lenStorage/2-self.f0shift-flen):int(self.lenStorage/2-self.f0shift+flen-1)] = 1;
   tmpMask0[int(self.lenStorage/2+self.f0shift-flen):int(self.lenStorage/2+self.f0shift+flen-1)] = 1;

   # f1 filter in frequency domain
   #mask1 = [f1loc(1)-400:f1loc(1)+399 f1loc(2)-400:f1loc(2)+399];
   tmpMask1 = np.zeros(lenStorage);
   tmpMask1[int(self.lenStorage/2-self.f1shift-flen):int(self.lenStorage/2-self.f1shift+flen-1)] = 1;
   tmpMask1[int(self.lenStorage/2+self.f1shift-flen):int(self.lenStorage/2+self.f1shift+flen-1)] = 1;


   #% generation of the frequency shifted sequence
   # symLengthInT = 40e-3;
   dt = 1/fs
   symSamples = np.arange(0,(symLengthInT),dt);
   self.lenSymbol = len(symSamples);
   reff0Boolean = np.cos(2*np.pi*f0*symSamples)>0;
   self.reff0 = [float(item == True) for item in reff0Boolean]
   reff1Boolean = np.cos(2*np.pi*f1*symSamples)>0;
   self.reff1 = [float(item == True) for item in reff1Boolean]
   #reff=[reff0; reff1]*2-1;

   self.symMatchedFilter = np.ones(int(self.lenSymbol/2))/(self.lenSymbol/2)


   self.mask00 = np.conj(np.fft.fftshift(np.fft.fft(self.reff0,lenStorage)))*tmpMask0;
   self.mask11 = np.conj(np.fft.fftshift(np.fft.fft(self.reff1,lenStorage)))*tmpMask1;


  def fskSymbolEstimate(self,inputSignal):
    #print("start")
    ## signal 0
    np.size(inputSignal)
    tmpSignal = np.fft.fftshift(np.fft.fft(inputSignal,self.lenStorage));
    tmp00 = tmpSignal*self.mask00; # masking and matching filtering with freq modulated signal

    g01=np.roll(tmp00,int(self.f0shift))
    g02=np.roll(tmp00,-int(self.f0shift))
    #plt.plot(g01)
    #plt.plot(g02)
    #plt.show()
    g03 = np.fft.ifft(np.fft.fftshift((g01+g02)*self.maskCenter))
    g03 = np.concatenate([g03,np.zeros(int(self.lenSymbol/2-1))])
    g04 = np.convolve(abs(g03), self.symMatchedFilter, mode='valid')
  
    # g01((f0shift+1):end)=tmp00(1:(end-f0shift));
    # g02(1:(end-f0shift))=tmp00((f0shift+1):end);
    # g03 = ifft(fftshift((g01+g02).*maskCenter));
    # g04 = movmean(abs(g03),lenSymbol/2);
  
    ## signal 1
    tmp10 = tmpSignal*self.mask11;

    g11=np.roll(tmp10,int(self.f1shift))
    g12=np.roll(tmp10,-int(self.f1shift))
    g13 = np.fft.ifft(np.fft.fftshift((g11+g12)*self.maskCenter))
    g13 = np.concatenate([g13,np.zeros(int(self.lenSymbol/2-1))])
    g14 = np.convolve(abs(g13), self.symMatchedFilter, mode='valid')
    #print(len(g14))
    
    # g11 = zeros(1,lenStorage);
    # g12 = zeros(1,lenStorage);
    # g11((f1shift+1):end)=tmp10(1:(end-f1shift));
    # g12(1:(end-f1shift))=tmp10((f1shift+1):end);
    # g13 = ifft(fftshift((g11+g12).*maskCenter));
    # g14 = movmean(abs(g13),lenSymbol/2);
  
    self.decVar = np.real(g04-g14);
 
    #plt.plot(self.decVar)
    #plt.show()
    #decVShaped = self.decVar.reshape(int(self.lenSymbol),int(self.lenStorage/self.lenSymbol));
    
    #for i1 in decVShaped:
    #  h1 = [int(item == True) for item in (i1>0)]
    #  print(h1)
    
    #self.decodedData = np.array([int(item == True) for item in (self.decVar[0:8080:80]<0)])
    
    #print(h1)
    #print((self.decVar[40:6400:80]))
    # sum(abs(decVShaped'))
  
    #return self.decodedData
    return self.decVar
  
  def bcDecoder(self):
  
    # decoder if present
    
    dataWithCRC = self.decodedData
    # check crc    

    return 0
  


