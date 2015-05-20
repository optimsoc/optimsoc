SCRIPTDIR=$(shell pkg-config --variable=buildscriptdir optimsoc-gzll)

all: hello.elf

hello.elf: hello.o

include $(SCRIPTDIR)/Makefile-app.inc
