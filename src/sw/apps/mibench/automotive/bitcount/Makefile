PROGRAM=bitcnts
OTHER_SRCS=bitcnt_1.c \
	bitcnt_2.c \
	bitcnt_3.c \
	bitcnt_4.c \
	bitfiles.c \
	bitstrng.c \
	bstr_i.c
LD_ADD=
CFLAGS=-O3 -std=gnu99 -Wall

BUILDSCRIPTS=$(shell pkg-config --variable=buildscriptdir optimsoc-system-dm-baremetal)
include $(BUILDSCRIPTS)/Makefile.inc

