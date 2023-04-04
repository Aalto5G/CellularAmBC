# Docker creation and code usage with docker

- [ ] Create docker file 
  
```
docker build -f Dockerfile -t bcreceiver-image .
```
  
- [ ] Find the usrp usb port id 
```
lsusb
```
  Example output with bus = 004 device == 079:\
  Bus 004 Device 079: ID 2500:0022 Ettus Research LLC Unknown
  
  
- [ ] Start docker in interactive mode with the access to the usb device 
```
docker run --name bcReceiver -t -i --device /dev/bus/usb/004/079 bcreceiver-image bash
```
- [ ] Start LTE receiver in the docker command line 
```
./rxSTM -f 486e6 -s 327AAA6 -g 45
```
-f  received channel frequency in Hz (default 806e6)\
-s  usrp device serial id\
-g  usrp receiver gain               (default 45)

- [ ] Start backscatter reader in another xterminal window

Starting interactive session inside the docker bcReceiver 
```
docker exec -it bcReceiver bash
```
  
  Inside docker go to Python code directory and start the backscatter receiver 
```
cd /Py/
./mainPython.py
```

