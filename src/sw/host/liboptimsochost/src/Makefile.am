pkginclude_HEADERS = optimsochost/liboptimsochost.h
lib_LTLIBRARIES = liboptimsochost.la

liboptimsochost_la_SOURCES =\
	liboptimsochost-private.h \
	liboptimsochost.c \
	log.c \
	backend_simtcp.c \
	backend_dbgnoc.c \
	backend_dbgnoc_tcp.c \
	backend_dbgnoc_usb.c

liboptimsochost_la_LDFLAGS = $(AM_LDFLAGS) $(LTLDFLAGS)
liboptimsochost_la_CPPFLAGS = $(AM_CPPFLAGS) -include $(top_builddir)/config.h
