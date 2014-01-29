libmcapi_LTLIBRARIES = libmcapi.la
libmcapidir = $(libdir)/virtualapps

libmcapi_la_SOURCES =\
    mcapi.c \
    mcapi_trans_simple/mcapi_trans_simple.c

libmcapi_la_LDFLAGS = $(AM_LDFLAGS) $(LTLDFLAGS)

libmcapi_la_CPPFLAGS = $(AM_CPPFLAGS) -Wall \
                       -I $(OPTIMSOC_NEWLIB)/optimsoc-virtualapps/include \
                       -I $(top_srcdir)/src/virtualapps/libmcapi/include/ \
                       -I $(top_srcdir)/src/baremetal/libmcapi/

mcapiincludedir = $(includedir)/virtualapps/mcapi
mcapiinclude_HEADERS = include/mca_config.h \
                       include/mca_impl_spec.h \
                       include/mca.h \
                       include/mcapi_impl_spec.h \
                       include/mcapi_trans.h \
                       include/mcapi.h

# prepare pkg-config file
share/%.pc: share/%.pc.in Makefile
	$(AM_V_GEN)$(MKDIR_P) share; \
	$(SED) \
	-e 's,@VERSION\@,$(VERSION),g' \
	-e 's,@prefix\@,$(prefix),g' \
	-e 's,@datadir\@,$(datadir),g' \
	-e 's,@libdir\@,$(libdir),g' \
	-e 's,@includedir\@,$(includedir),g' \
	< $< > $@ || rm $@

pkgconfigdir = $(datadir)/pkgconfig
pkgconfig_DATA = share/optimsoc-virtualapps-mcapi.pc

EXTRA_DIST = share/optimsoc-virtualapps-mcapi.pc.in
CLEANFILES = share/optimsoc-virtualapps-mcapi.pc