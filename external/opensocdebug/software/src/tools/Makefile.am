# build a convenience library libcliutil
# This avoids including these source files multiple times from different tools,
# which breaks the make dist* tools from automake.
SUBDIRS = .
noinst_LTLIBRARIES = libcliutil.la
libcliutil_la_SOURCES = dictionary.c iniparser.c argtable3.c

SUBDIRS += osd-host-controller

if USE_GLIP
SUBDIRS += \
	osd-device-gateway \
	osd-target-run
endif
