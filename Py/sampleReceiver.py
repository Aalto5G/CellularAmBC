#!/usr/bin/python3

# Executes UDP server 
#  read samples and puts them into a buffer

import socket
import struct
import numpy as np

# import sys 
# import os
# sys.path.append(os.path.abspath("../FSK"))
# from fskReceiver import *

from scipy.io import loadmat

class sampleReceiver():
  def __init__(self,localIP,localPort,bufferSize):
    print("init called")
    self.localIP = localIP
    self.localPort = localPort
    self.bufferSize = bufferSize
    
    self.udpDataBuffer = np.zeros(140)
    
    
    self.lenSamplesInOneUDP = 20
    self.storageLengthFilteredDS = self.lenSamplesInOneUDP * 500
    self.downSampledBuffer = np.zeros(self.storageLengthFilteredDS)
    
    # annots = loadmat('../filterCoeffCut2k_Sampl14k.mat')
    # firstFilterNum = annots['Num'][0]
    annots = loadmat('./LP_FIR_1000_1400_fs14000.mat')
    firstFilterNum = annots['lp_fir_ds'][0]
    self.lenFFT = 512
    lenCircularBuffer = self.lenFFT;
    # np.conjugate
    self.lp_filter_rx_ch_estimation_inF = np.conj(np.fft.fft(firstFilterNum,self.lenFFT))  # in frequency domain

    self.tmpTailStore = np.zeros(self.lenFFT,dtype=complex)
    self.tmp_rx_pilots_filtered = np.zeros(self.lenFFT)
    


    self.f0 = 250
    self.f1 = 625
    self.fs = 2e3

    symLenInT = 40e-3;


#    self.fskRx = fskReceiver(self.f0,self.f1,self.fs,self.storageLengthFilteredDS,symLenInT)

    self.readCount = 0
    self.openPort()


  def openPort(self):
    self.UDPServerSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
    self.UDPServerSocket.bind((self.localIP, self.localPort))
    #print("UDP server up and listening")
    return 0


  def readPort(self):
    self.readCount = self.readCount + 1
        
    bytesAddressPair = self.UDPServerSocket.recvfrom(self.bufferSize)
    message = bytesAddressPair[0]
    # address = bytesAddressPair[1]

#    print(len(message))
#    print(message[0])
    if message[0] == 0x02:
      tmp1 = message[1:3]
      tmpNr = struct.unpack('<H', tmp1)  # tmpNr: How many subframe follows?
      # print(tmpNr)
# old version working with Matlab data structure
      locPtr = 1
      tmpElemN = np.arange(20)
      for n in tmpElemN:
        locPtr = locPtr + 2
        tmpLenN2 = np.arange(7)
        
        for n2 in tmpLenN2:
          tmpNr1 = struct.unpack('d',message[locPtr:(locPtr+8)])
          locPtr = locPtr + 8
          self.udpDataBuffer[n * 7 + n2] = tmpNr1[0]

      #print(self.udpDataBuffer)
# version for a new structure 
#      locPtr = 3
#      tmpElemN = np.arange(tmpNr[0])
#      for n in tmpElemN:
#        tmp1 = message[(locPtr + 1):(locPtr + 3)]
#        tmpNr = struct.unpack('<H', tmp1)
#        locPtr = locPtr + 3
#        tmpElemN2 = np.arange(7)
#        for n2 in tmpElemN2:
#          tmpNr1 = struct.unpack('d', message[locPtr:(locPtr + 8)])
#          # print(tmpNr1)
#          locPtr = locPtr + 8
#          self.udpDataBuffer[n * 7 + n2] = tmpNr1[0]
#       #count = count + 1
#    #return testBuffer
    
  def filterRxPilots(self,rxBuffer):
    
    testBufferF = np.fft.fft(rxBuffer,self.lenFFT)

    # filtering
    testBufferFilteredF = testBufferF*self.lp_filter_rx_ch_estimation_inF

    #plt.plot(abs(testBufferFilteredF))
    #plt.show()

    self.tmp_rx_pilots_filtered = np.fft.fftshift(np.fft.ifft(testBufferFilteredF,self.lenFFT))

    return 0

  ###################################################
  # inserting data into buffer  
  def insertFilteredRxPilotsIntoRxBuffer(self,inBufferFiltered):
 
    tmpTail2 = self.tmpTailStore+inBufferFiltered  # combines the last stored data and the new data
    lenSamplesInOneUDP = self.lenSamplesInOneUDP
    self.downSampledBuffer[:lenSamplesInOneUDP] = 0; # clears the beginning of the "circular buffer"    
#    self.downSampledBuffer[:lenSamplesInOneUDP]=abs(tmpTail2[0:140:7]) # inserts new data to the buffer
    self.downSampledBuffer = np.roll(self.downSampledBuffer,-lenSamplesInOneUDP) #circular shifts the data to be newest ie. at the end of the buffer
# add new samples to the end of the buffer
    #print(len(self.downSampledBuffer[(self.storageLengthFilteredDS-53):]))
    self.downSampledBuffer[(self.storageLengthFilteredDS-53):] = self.downSampledBuffer[(self.storageLengthFilteredDS-53):]+np.real(inBufferFiltered[1:(512-140):7])

    # stores the end of the buffer where filtered data is
    tmpB1 = np.zeros(self.lenFFT,dtype=complex)
    tmpB1[(512-140):] = inBufferFiltered[(512-140):]; # pick up the end of the filtered data buffer
    self.tmpTailStore = np.roll(tmpB1,-140) # shift the tail into beginning of the buffer and store it for to be added to the next received buffer  
    return 0
        
  ####################################################
  # fsk detection
#  def fskDetection(self,downSampledBuffer):
   
    #plt.plot(np.real(downSampledBuffer))
    #plt.show()

    # fsk symbol receiver
#    self.decisionVariable = self.fskRx.fskSymbolEstimate(downSampledBuffer)
      
#    return self.decidionVariable

  ####################################################
  # receiver samples
  def receiveSamples(self):
      self.readPort()
      #print(self.udpDataBuffer)
      self.filterRxPilots(self.udpDataBuffer)
      self.insertFilteredRxPilotsIntoRxBuffer(self.tmp_rx_pilots_filtered)
  
