SUBDIRS = . tools

pkginclude_HEADERS = $(top_srcdir)/include/libglip.h

lib_LTLIBRARIES = libglip.la

libglip_la_SOURCES =\
	cbuf.c \
	glip.c \
	util.c

if BACKEND_UART
libglip_la_SOURCES += backend_uart/sw/backend_uart.c
endif

if BACKEND_CYPRESSFX2
libglip_la_SOURCES += backend_cypressfx2/sw/backend_cypressfx2.c
AM_CFLAGS += $(libusb_CFLAGS)
AM_LDFLAGS += $(libusb_LIBS)
endif

if BACKEND_TCP
libglip_la_SOURCES += backend_tcp/sw/backend_tcp.c
endif

if BACKEND_JTAG
libglip_la_SOURCES += backend_jtag/sw/backend_jtag.c

backend_jtag_dir = $(pkgdatadir)/backend_jtag
openocd_dir = $(backend_jtag_dir)/openocd
openocd__DATA = $(top_srcdir)/src/backend_jtag/openocd/glip.cfg
endif

libglip_la_LDFLAGS = $(AM_LDFLAGS) $(LTLDFLAGS)
libglip_la_CPPFLAGS = $(AM_CPPFLAGS) \
                      -I $(top_srcdir)/include \
                      -DPKGDATADIR='"$(pkgdatadir)"'
