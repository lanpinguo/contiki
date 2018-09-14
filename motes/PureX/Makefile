CONTIKI = ../..
CONTIKI_PROJECT = PureX

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"
APPS = serial-shell
CONTIKI_WITH_RIME = 1

all: $(CONTIKI_PROJECT) hexImage

hexImage:
	arm-none-eabi-objcopy -O ihex $(CONTIKI_PROJECT).$(TARGET) $(CONTIKI_PROJECT).hex

include $(CONTIKI)/Makefile.include
