CC = gcc
CFLAGS = -Wall
PRU_ASM = pasm
DTC = dtc

all: gps_timer.bin gps_timer gps_timer.dtbo

gps_timer.dtbo: gps_timer.dts
	$(DTC) -O dtb -o gps_timer-00A0.dtbo -b 0 -@ gps_timer.dts

gps_timer.bin: gps_timer.p
	$(PRU_ASM) -b gps_timer.p

pruFn.o: pruFn.c __pruFn.h
	$(CC) $(CFLAGS) -c -o pruFn.o pruFn.c

gps.o: adafruitUltimateGPS.c adafruitUltimateGPS.h
	$(CC) $(CFLAGS) -c -o gps.o adafruitUltimateGPS.c

audio.o: audio.c audio.h
	$(CC) $(CFLAGS) -c -o audio.o audio.c

gps_timer.o: gps_timer.c __pruFn.h adafruitUltimateGPS.h
	$(CC) $(CFLAGS) -c -o gps_timer.o gps_timer.c

gps_timer: gps_timer.o pruFn.o gps.o audio.o
	$(CC) -lpthread -lprussdrv -o gps_timer gps_timer.o pruFn.o gps.o audio.o -lasound -liofunc
clean:
	rm -f gps_timer *.o *.bin *.dtbo

install: gps_timer-00A0.dtbo gps_timer.bin gps_timer
	cp gps_timer-00A0.dtbo /lib/firmware
	echo gps_timer > /sys/devices/bone_capemgr.9/slots
	cat /sys/devices/bone_capemgr.9/slots 
	cat /sys/kernel/debug/pinctrl/44e10800.pinmux/pins | grep 830
	cat /sys/kernel/debug/pinctrl/44e10800.pinmux/pins | grep 838
	cat /sys/kernel/debug/pinctrl/44e10800.pinmux/pins | grep 87c