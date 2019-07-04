CONTIKI_PROJECT = mxchip-shell

PROJECTDIRS = $(CONTIKI)/platform/mxchip/apps

APPS = serial-shell

PROJECT_SOURCEFILES = shell-sensors.c

all: $(CONTIKI_PROJECT) hex_image

hex_image:
	arm-none-eabi-objcopy -O ihex $(CONTIKI_PROJECT).mxchip $(CONTIKI_PROJECT).hex

CONTIKI = ../../..
CONTIKI_WITH_RIME = 1
include $(CONTIKI)/Makefile.include
