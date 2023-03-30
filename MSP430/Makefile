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

# GCC_DIR = $(abspath $(dir $(lastword $(MAKEFILE)))/../../bin)
GCC_DIR = /usr/local/msp430-gcc/bin
#SUPPORT_FILE_DIRECTORY = $(abspath $(dir $(lastword $(MAKEFILE)))/../../include)
SUPPORT_FILE_DIRECTORY = /usr/local/msp430-gcc/include

DEVICE  = MSP430G2553
CC      = $(GCC_DIR)/msp430-elf-gcc
GDB     = $(GCC_DIR)/msp430-elf-gdb

CFLAGS = -I $(SUPPORT_FILE_DIRECTORY) -mmcu=$(DEVICE) -Og -Wall -g
LFLAGS = -L $(SUPPORT_FILE_DIRECTORY) -Wl,-Map,$(MAP),--gc-sections 

multiFreq: multiFreq.o 
	$(CC) $(CFLAGS)  -L $(SUPPORT_FILE_DIRECTORY) -W $? -o multiFreq.out
	mspdebug rf2500 "prog multiFreq.out"

all: ${OBJECTS}
	$(CC) $(CFLAGS) $(LFLAGS) $? -o $(DEVICE).out
	mspdebug rf2500 "prog $(DEVICE).out"

clean: 
	$(RM) $(OBJECTS)
	$(RM) $(MAP)
	$(RM) *.out
	
debug: all
	$(GDB) $(DEVICE).out

compile: ${OBJECTS}
	$(CC) $(CFLAGS) $(LFLAGS) $? -o $(DEVICE).out
