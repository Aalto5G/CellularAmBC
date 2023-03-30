This file descirbe how to compile C and write to the MSP430 flash.
In this file case, we should use
```sh
cd ./examples/msp430g2553
make MultiFreq
```
and write flash as
```sh
mspdebug rf2500 "prog MultiFreq.out"
```

# msp430-elf-gcc and mspdebug

I think Arduino time has some skew, that because it not bare C. To code C, we have to prepare [compiler](https://www.ti.com/tool/MSP430-GCC-OPENSOURCE) and gdb to write binary file to MSP430 flash.\\

## compile

1. download from website https://www.ti.com/tool/MSP430-GCC-OPENSOURCE
2. I install at /usr/local/msp430-gcc/
3. Try blink example in the msp430-gcc.

```sh
cd ./examples/msp430g2553
make
```

Then get the binary file `MSP430G2553.out`.\
Makefile

```make
OBJECTS=blink.o
MAP=blink.map
MAKEFILE=Makefile

ifeq ($(OS),Windows_NT)
	ifeq ($(shell uname -o),Cygwin)
		RM= rm -rf
	else
		RM= del /q
	endif
else
	RM= rm -rf
endif

GCC_DIR = $(abspath $(dir $(lastword $(MAKEFILE)))/../../bin)
SUPPORT_FILE_DIRECTORY = $(abspath $(dir $(lastword $(MAKEFILE)))/../../include)

DEVICE  = MSP430G2553
CC      = $(GCC_DIR)/msp430-elf-gcc
GDB     = $(GCC_DIR)/msp430-elf-gdb

CFLAGS = -I $(SUPPORT_FILE_DIRECTORY) -mmcu=$(DEVICE) -Og -Wall -g
LFLAGS = -L $(SUPPORT_FILE_DIRECTORY) -Wl,-Map,$(MAP),--gc-sections 

all: ${OBJECTS}
	$(CC) $(CFLAGS) $(LFLAGS) $? -o $(DEVICE).out

clean: 
	$(RM) $(OBJECTS)
	$(RM) $(MAP)
	$(RM) *.out
	
debug: all
	$(GDB) $(DEVICE).out
```

blink.c

```c
//***************************************************************************************
//  MSP430 Blink the LED Demo - Software Toggle P1.2
//
//  Description; Toggle P1.2 by xor'ing P1.2 inside of a software loop.
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//                MSP430x5xx
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P1.2|-->LED
//
//  Texas Instruments, Inc
//  July 2011
//***************************************************************************************

#include <msp430.h>				

void main(void) {
	WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer
	P1DIR |= 0x04;					// Set P1.2 to output direction

	for(;;) {
		volatile unsigned int i;	// volatile to prevent optimization

		P1OUT ^= 0x04;				// Toggle P1.2 using exclusive-OR

		i = 10000;					// SW Delay
		do i--;
		while(i != 0);
	}
}
```

## write flash

install mspdebug

```shell
sudo apt-get remove mspdebug
```

check device connection

```sh
lsusb | grep Texas
mspdebug --usb
```

write binary code to flash\
method 1:

```sh
$ mspdebug rf2500

MSPDebug version 0.22 - debugging tool for MSP430 MCUs
Copyright (C) 2009-2013 Daniel Beer <dlbeer@gmail.com>
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

Trying to open interface 1 on 011
Initializing FET...
FET protocol version is 30394216
Set Vcc: 3000 mV
Configured for Spy-Bi-Wire
Device ID: 0x2553
  Code start address: 0xc000
  Code size         : 16384 byte = 16 kb
  RAM  start address: 0x200
  RAM  end   address: 0x3ff
  RAM  size         : 512 byte = 0 kb
Device: MSP430G2553/G2403
Number of breakpoints: 2
fet: FET returned NAK
warning: device does not support power profiling
Chip ID data: 25 53

Available commands:
    =           erase       isearch     power       save_raw    simio       
    alias       exit        load        prog        set         step        
    break       fill        load_raw    read        setbreak    sym         
    cgraph      gdb         md          regs        setwatch    verify      
    delbreak    help        mw          reset       setwatch_r  verify_raw  
    dis         hexout      opt         run         setwatch_w  

Available options:
    color                       gdb_loop                    
    enable_bsl_access           gdbc_xfer_size              
    enable_locked_flash_access  iradix                      
    fet_block_size              quiet                       
    gdb_default_port            

Type "help <topic>" for more information.
Use the "opt" command ("help opt") to set options.
Press Ctrl+D to quit.

(mspdebug) prog MSP430G2553.out 
Erasing...
Programming...
Writing    2 bytes at fffe [section: __reset_vector]...
Writing   44 bytes at c000 [section: .text]...
Done, 46 bytes total
```

method 2:

```sh
mspdebug rf2500 "prog MSP430G2553.out"
```
