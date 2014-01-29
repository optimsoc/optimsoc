libruntime_LTLIBRARIES = libruntime.la
libruntimedir = $(libdir)/virtualapps

libruntime_la_SOURCES = runtime.c
    
libruntime_la_LDFLAGS = $(AM_LDFLAGS) $(LTLDFLAGS)

libruntime_la_CPPFLAGS = $(AM_CPPFLAGS) \
	-I $(OPTIMSOC_NEWLIB)/optimsoc-virtualapps/include \
	-I $(top_srcdir)/../../../shared_header
	-Wall

runtimeincludedir = $(includedir)/virtualapps
runtimeinclude_HEADERS = include/optimsoc-runtime.h

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
pkgconfig_DATA = share/optimsoc-virtualapps-runtime.pc

EXTRA_DIST = share/optimsoc-virtualapps-runtime.pc.in
CLEANFILES = share/optimsoc-virtualapps-runtime.pc