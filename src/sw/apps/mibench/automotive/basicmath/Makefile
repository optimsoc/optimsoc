PROGRAM=basicmath_small
OTHER_SRCS=rad2deg.c \
	cubic.c \
	isqrt.c
LD_ADD=-lm
CFLAGS=-O3 -std=gnu99 -Wall

BUILDSCRIPTS=$(shell pkg-config --variable=buildscriptdir optimsoc-system-dm-baremetal)
include $(BUILDSCRIPTS)/Makefile.inc

