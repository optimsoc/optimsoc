pkginclude_HEADERS =\
	include/osd/osd.h \
	include/osd/reg.h \
	include/osd/packet.h \
	include/osd/module.h \
	include/osd/hostmod.h \
	include/osd/hostctrl.h \
	include/osd/gateway.h \
	include/osd/cl_mam.h \
	include/osd/cl_scm.h \
	include/osd/cl_stm.h \
	include/osd/cl_ctm.h \
	include/osd/cl_cdm.h \
	include/osd/memaccess.h \
	include/osd/systracelogger.h \
	include/osd/coretracelogger.h \
	include/osd/cl_dem_uart.h \
	include/osd/terminal.h

lib_LTLIBRARIES = libosd.la

libosd_la_SOURCES =\
	log.c \
	module.c \
	packet.c \
	hostmod.c \
	hostctrl.c \
	worker.c \
	util.c \
	gateway.c \
	cl_mam.c \
	cl_scm.c \
	cl_stm.c \
	cl_ctm.c \
	cl_cdm.c \
	cl_dem_uart.c \
	memaccess.c \
	systracelogger.c \
	coretracelogger.c \
	terminal.c

libosd_la_CFLAGS = $(AM_CFLAGS)

libosd_la_LDFLAGS = \
	$(AM_LDFLAGS) \
	$(LTLDFLAGS) \
    -fvisibility=hidden

libosd_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(srcdir)/include \
	-include $(top_builddir)/config.h

if USE_GLIP
   pkginclude_HEADERS += include/osd/gateway_glip.h
   libosd_la_SOURCES += gateway_glip.c
   libosd_la_LDFLAGS += ${libglip_LIBS}
   libosd_la_CFLAGS += ${libglip_CFLAGS}
endif
