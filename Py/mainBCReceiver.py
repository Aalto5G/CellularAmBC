#!/usr/bin/python3

import socket
import struct
import numpy as np

import sys 
import os
sys.path.append(os.path.abspath("./FSK"))
from fskReceiver import *

sys.path.append(os.path.abspath("./GUI"))
from drawSpaceInvader import *

sys.path.append(os.path.abspath("./"))
from sampleReceiver import *

from scipy.io import loadmat


localIP     = "127.0.0.1"
localPort   = 8090
bufferSize  = 1183

f0 = 250
f1 = 625
fs = 2e3

symLenInT = 40e-3;

pilots = np.array([0,0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0])
# dataColFirst = np.array([0 0 0 1 1 0 0 1 1 0 1 1 1 0 1 0 0 1 1 0 1 1 0 0 0 1 0 0 1 0 0 0 0 1 1 1 1 0 0 0 0 1 1 1 1 0 0 0 0 1 1 0 1 0 0 0 0 1 0 0 1 1 0 0 1 0 1 1 1 0 1 0 0 0 0 1 1 0 0 1]);
dataRowFirst = np.array([0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1,  0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1]);

refData = np.concatenate([pilots,dataRowFirst]); 


def mainRun():
  srx = sampleReceiver(localIP,localPort,bufferSize)
  fskRx = fskReceiver(f0,f1,fs,srx.storageLengthFilteredDS,symLenInT)

  count = 0
  while(True):
    for i1 in np.arange(4):
      srx.receiveSamples()
    
    decVar = fskRx.fskSymbolEstimate(srx.downSampledBuffer)
 
    decVar2= np.reshape(decVar[0:(101*80)],[101,80]).transpose();
    
    tmpVal = np.sum(abs(decVar2),1)
    
    #print(len(tmpVal))
    l3 = np.where(tmpVal == tmpVal.max())
    

    h1 = decVar[0:8080:80];
    #print(decVar2)
    #print(len(decVar2[0]))
    #for i1 in np.arange(101):
    #  print("{} {} |".format(h1[i1],decVar2[0][i1]))
    
    #decodedData = np.array([int(item == True) for item in (decVar[0:8080:80]<0)])
    #print("est {}".format(l3[0][0]))
    #print(decVar2[l3[0][0]])
    #print(decVar[0:8080:80])
    decodedData = np.array([int(item == True) for item in (decVar2[l3[0][0]]<0)])

    errSumAll = 0
    for rxData,txData in zip(decodedData[:101],refData):
       if rxData!=txData:
         errSumAll = errSumAll+1
    
    errSumPilots = 0
    for rxData,txData in zip(decodedData[:21],pilots):
       if rxData!=txData:
         errSumPilots = errSumPilots+1
    
    #if errSumPilots < 4:
    #  print("{} total errors in pilots: {}".format(count,errSumPilots))

    errSumData = 0
    for rxData,txData in zip(decodedData[21:101],dataRowFirst):
       if rxData!=txData:
         errSumData = errSumData+1
    
    if errSumData < 10:
      print("time: {} total errors: {} pilot errors: {} data errors: {}".format(count,errSumAll,errSumPilots,errSumData))
      #print(str(count)+" "+str(errSumPilots)+" total errors: "+str(errSumAll) +" errors in data: "+str(errSumData))
    
    count = count+1*0.08
       
if __name__ == '__main__':

  mainRun()

