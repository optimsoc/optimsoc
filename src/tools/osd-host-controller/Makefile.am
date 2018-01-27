bin_PROGRAMS = osd-host-controller

osd_host_controller_LDADD = \
	../libcliutil.la \
	../../libosd/libosd.la

AM_LDFLAGS += \
	${libczmq_LIBS}

AM_CFLAGS += \
	-I$(top_srcdir)/src/libosd/include \
	-include $(top_builddir)/config.h \
	-I$(srcdir)/../common \
	${libczmq_CFLAGS}

osd_host_controller_SOURCES = \
	osd-host-controller.c
