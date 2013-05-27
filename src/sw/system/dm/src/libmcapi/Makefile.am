lib_LTLIBRARIES = libmcapi-simple-bare.la libmcapi-simple-runtime.la

libmcapi_simple_bare_la_SOURCES =\
    mcapi.c \
    mcapi_trans_simple/mcapi_trans_simple.c
libmcapi_simple_bare_la_LDFLAGS = $(AM_LDFLAGS) $(LTLDFLAGS)
libmcapi_simple_bare_la_CPPFLAGS = $(AM_CPPFLAGS) -I $(OPTIMSOC_NEWLIB)/include \
    -I $(top_srcdir)/src/libbaremetal/include \
    -I $(top_srcdir)/src/libmcapi/include/ \
    -Wall

libmcapi_simple_runtime_la_SOURCES =\
    mcapi.c \
    mcapi_trans_simple/mcapi_trans_simple.c
libmcapi_simple_runtime_la_LDFLAGS = $(AM_LDFLAGS) $(LTLDFLAGS)
libmcapi_simple_runtime_la_CPPFLAGS = $(AM_CPPFLAGS) \
    -I $(OPTIMSOC_NEWLIB)/include \
    -I $(top_srcdir)/src/libbaremetal/include \
    -I $(top_srcdir)/src/libmcapi/include/ \
    -I $(top_srcdir)/src/libruntime/include/ \
    -D RUNTIME -Wall

mcapiincludedir = $(includedir)/mcapi
mcapiinclude_HEADERS = include/mcapi.h include/mca_config.h \
	include/mca_impl_spec.h include/mca.h include/mcapi_impl_spec.h \
	include/mcapi_trans.h  

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
pkgconfig_DATA = share/optimsoc-mcapi-simple-bare.pc \
                 share/optimsoc-mcapi-simple-runtime.pc
EXTRA_DIST = share/optimsoc-mcapi-simple-bare.pc.in \
             share/optimsoc-mcapi-simple-runtime.pc.in
CLEANFILES = share/optimsoc-mcapi-simple-bare.pc \
             share/optimsoc-mcapi-simple-runtime.pc