# Name: Makefile
# Project: Remote Sensor
# Author: Christian Starkjohann, modified for LCD2USB by Till Harbaum
# Creation Date: 2005-03-20
# Tabsize: 4
# Copyright: (c) 2005 by OBJECTIVE DEVELOPMENT Software GmbH
# License: Proprietary, free under certain conditions. See Documentation.
# This Revision: $Id: Makefile.avrusb,v 1.1 2007/01/14 12:12:27 harbaum Exp $

# DEFINES += -DBWCT_COMPAT 
# DEFINES += -DDEBUG_LEVEL=1
DEVICE = atmega8
FUSEH = 0xc0
FUSEL = 0x9f

DEFINES += -DF_CPU=12000000
COMPILE = avr-gcc -Wall -O2 -Iusbdrv -Ilcd -I. -mmcu=$(DEVICE) $(DEFINES)
AVRDUDE = avrdude -c usbasp -p $(DEVICE)
OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o usbdrv/oddebug.o main.o lcd/lcd.o

# symbolic targets:
all:	firmware-avrusb.hex

.c.o:
	$(COMPILE) -c $< -o $@

.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

.c.s:
	$(COMPILE) -S $< -o $@

flash:	all
	avrdude -c usbasp -p $(DEVICE) -U lfuse:w:$(FUSEL):m -U hfuse:w:$(FUSEH):m -U flash:w:firmware-avrusb.hex

flash-nodep:
	avrdude -c usbasp -p $(DEVICE) -U lfuse:w:$(FUSEL):m -U hfuse:w:$(FUSEH):m -U flash:w:firmware-avrusb.hex

# Fuse high byte:
# 0xc9 = 1 1 0 0   1 0 0 1 <-- BOOTRST (boot reset vector at 0x0000)
#        ^ ^ ^ ^   ^ ^ ^------ BOOTSZ0
#        | | | |   | +-------- BOOTSZ1
#        | | | |   + --------- EESAVE (don't preserve EEPROM over chip erase)
#        | | | +-------------- CKOPT (full output swing)
#        | | +---------------- SPIEN (allow serial programming)
#        | +------------------ WDTON (WDT not always on)
#        +-------------------- RSTDISBL (reset pin is enabled)
# Fuse low byte:
# 0x9f = 1 0 0 1   1 1 1 1
#        ^ ^ \ /   \--+--/
#        | |  |       +------- CKSEL 3..0 (external >8M crystal)
#        | |  +--------------- SUT 1..0 (crystal osc, BOD enabled)
#        | +------------------ BODEN (BrownOut Detector enabled)
#        +-------------------- BODLEVEL (2.7V)
fuse:
	$(AVRDUDE) -U hfuse:w:$(FUSEH):m -U lfuse:w:$(FUSEL):m


clean:
	rm -f firmware-avrusb.hex firmware-avrusb.lst firmware-avrusb.obj firmware-avrusb.cof firmware-avrusb.list firmware-avrusb.map firmware-avrusb.eep.hex firmware-avrusb.bin lcd/*.o *.o usbdrv/*.o firmware-avrusb.s usbdrv/oddebug.s usbdrv/usbdrv.s

# file targets:
firmware-avrusb.bin:	$(OBJECTS)
	$(COMPILE) -o firmware-avrusb.bin $(OBJECTS)

firmware-avrusb.hex:	firmware-avrusb.bin
	rm -f firmware-avrusb.hex firmware-avrusb.eep.hex
	avr-objcopy -j .text -j .data -O ihex firmware-avrusb.bin firmware-avrusb.hex
	./checksize firmware-avrusb.bin 6144 960
# do the checksize script as our last action to allow successful compilation
# on Windows with WinAVR where the Unix commands will fail.

avrdude: firmware-avrusb.hex
	avrdude -c usbasp -p $(DEVICE) -U lfuse:w:$(FUSEL):m -U hfuse:w:$(FUSEH):m -U flash:w:firmware-avrusb.hex

avrdude-nodep:
	avrdude -c usbasp -p $(DEVICE) -U lfuse:w:$(FUSEL):m -U hfuse:w:$(FUSEH):m -U flash:w:firmware-avrusb.hex

disasm:	firmware-avrusb.bin
	avr-objdump -d firmware-avrusb.bin

cpp:
	$(COMPILE) -E main.c
