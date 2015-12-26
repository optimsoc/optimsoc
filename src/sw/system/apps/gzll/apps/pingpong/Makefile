SCRIPTDIR=$(shell pkg-config --variable=buildscriptdir optimsoc-gzll)

all: sender.elf receiver.elf

sender.elf: sender.o
receiver.elf: receiver.o

include $(SCRIPTDIR)/Makefile-app.inc
