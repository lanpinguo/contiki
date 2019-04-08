CONTIKI = ../..
CONTIKI_PROJECT = PureX

TARGET = pure-cc2538

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
APPS = serial-shell
CONTIKI_WITH_RIME = 1
SOURCE_LDSCRIPT = ./cc2538_boot.lds

all: $(CONTIKI_PROJECT) hexImage

hexImage:
	arm-none-eabi-objcopy -O ihex $(CONTIKI_PROJECT).$(TARGET) $(CONTIKI_PROJECT).hex

include $(CONTIKI)/Makefile.include
