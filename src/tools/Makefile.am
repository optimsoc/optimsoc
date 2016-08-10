
LDADD = ../libglip.la

bin_PROGRAMS = glip_loopback_measure

# XXX: get from glip pkgconfig!
if BACKEND_CYPRESSFX2
AM_CFLAGS += $(libusb_CFLAGS)
AM_LDFLAGS += $(libusb_LIBS)
endif

glip_loopback_measure_LDFLAGS = $(AM_LDFLAGS)
glip_loopback_measure_CFLAGS  = $(AM_CFLAGS) -I$(top_srcdir)/include
glip_loopback_measure_SOURCES = loopback_measure.c \
                                options.c


