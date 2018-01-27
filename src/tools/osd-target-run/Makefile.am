bin_PROGRAMS = osd-target-run

osd_target_run_LDADD = \
	../libcliutil.la \
	../../libosd/libosd.la

AM_LDFLAGS += \
	${libczmq_LIBS} \
	${libglip_LIBS}

AM_CFLAGS += \
	-I$(top_srcdir)/src/libosd/include \
	-include $(top_builddir)/config.h \
	${libczmq_CFLAGS} \
	${libglip_CFLAGS}

osd_target_run_SOURCES = \
	osd-target-run.c
