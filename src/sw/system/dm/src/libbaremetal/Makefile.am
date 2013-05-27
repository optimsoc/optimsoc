lib_LTLIBRARIES = libbaremetal.la

libbaremetal_la_SOURCES =\
    cas.c \
    critical.c \
    dma.c \
    mp_simple.c \
    mutex.c \
    uart.c 

libbaremetal_la_LDFLAGS = $(AM_LDFLAGS) $(LTLDFLAGS)
libbaremetal_la_CPPFLAGS = $(AM_CPPFLAGS) -I $(OPTIMSOC_NEWLIB)/include -Wall
include_HEADERS = include/optimsoc-baremetal.h

scriptdir = $(datadir)/build/baremetal
script_DATA = share/create_linker_script.sh \
              share/link.ld.in \
              share/Makefile.inc \
              share/create_pgas_binary.sh

# install the scripts with executable permissions
install-data-hook:
	chmod +x $(DESTDIR)$(scriptdir)/create_linker_script.sh
	chmod +x $(DESTDIR)$(scriptdir)/create_pgas_binary.sh

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
pkgconfig_DATA = share/optimsoc-system-dm-baremetal.pc
EXTRA_DIST = share/optimsoc-system-dm-baremetal.pc.in
CLEANFILES = share/optimsoc-system-dm-baremetal.pc

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

