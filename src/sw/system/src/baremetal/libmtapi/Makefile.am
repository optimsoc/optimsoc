libmtapi_LTLIBRARIES = libmtapi.la
libmtapidir = $(libdir)/baremetal

libmtapi_la_SOURCES =\
    mtapi.c \
    mtapi_runtime.c \
    mtapi_impl_spec.c

libmtapi_la_LDFLAGS = $(AM_LDFLAGS) $(LTLDFLAGS)
libmtapi_la_CPPFLAGS = $(AM_CPPFLAGS) -Wall \
                       -I $(OPTIMSOC_NEWLIB)/optimsoc-baremetal/include \
                       -I $(top_srcdir)/src/baremetal/libbaremetal/include \
                       -I $(top_srcdir)/src/baremetal/libruntime/include \
                       -I $(top_srcdir)/src/baremetal/libmcapi/include \
                       -I $(top_srcdir)/src/baremetal/libmtapi/include

mtapiincludedir = $(includedir)/baremetal/mtapi
mtapiinclude_HEADERS = include/mtapi.h include/mtapi_impl_spec.h \
	include/mtapi_runtime.h include/mtapi_optimsoc_spec.h

scriptdir = $(datadir)/build/mtapi
script_DATA = share/create_linker_script.sh \
              share/link.ld.in \
              share/Makefile.inc \
              share/create_pgas_binary.sh

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
pkgconfig_DATA = share/optimsoc-baremetal-mtapi.pc
EXTRA_DIST = share/optimsoc-baremetal-mtapi.pc.in
CLEANFILES = share/optimsoc-baremetal-mtapi.pc

if OR32OBJDUMPSET
MYOR32OBJDUMP = $(OR32OBJDUMP)
else
MYOR32OBJDUMP = $(subst gcc,objdump,$(wordlist 1,1,$(CC)))
endif

if OR32OBJCOPYSET
MYOR32OBJCOPY = $(OR32OBJCOPY)
else
MYOR32OBJCOPY = $(subst gcc,objcopy,$(wordlist 1,1,$(CC)))
endif

share/Makefile.inc: share/Makefile.inc.in Makefile
	$(AM_V_GEN)$(MKDIR_P) share; \
	$(SED) -e 's,@OR32-CC@,$(CC),g' -e 's,@OR32-OBJDUMP@,$(MYOR32OBJDUMP),g' \
	-e 's,@OR32-OBJCOPY@,$(MYOR32OBJCOPY),g' < $< > $@ || rm $@

CLEANFILES += share/Makefile.inc

share/create_linker_script.sh: share/create_linker_script.sh.in Makefile 
	$(AM_V_GEN)$(MKDIR_P) share; \
	$(SED) -e 's,@OR32-CC@,$(CC),g' -e 's,@OR32-OBJDUMP@,$(MYOR32OBJDUMP),g' \
	-e 's,@OR32-OBJCOPY@,$(MYOR32OBJCOPY),g' < $< > $@ || rm $@

CLEANFILES += share/create_linker_script.sh	

share/create_pgas_binary.sh: share/create_pgas_binary.sh.in Makefile 
	$(AM_V_GEN)$(MKDIR_P) share; \
	$(SED) -e 's,@OR32-CC@,$(CC),g' -e 's,@OR32-OBJDUMP@,$(MYOR32OBJDUMP),g' \
	-e 's,@OR32-OBJCOPY@,$(MYOR32OBJCOPY),g' < $< > $@ || rm $@
	
CLEANFILES += share/create_pgas_binary.sh
