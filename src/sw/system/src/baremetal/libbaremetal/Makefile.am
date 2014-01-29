# Copyright (c) 2012-2013 by the author(s)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# Author(s):
#   Stefan Wallentowitz <stefan.wallentowitz@tum.de>

libbaremetal_LTLIBRARIES = libbaremetal.la
libbaremetaldir = $(libdir)/baremetal

libbaremetal_la_SOURCES =\
    cas.c \
    critical.c \
    dma.c \
    mp_simple.c \
    mutex.c \
    uart.c \
    timer.c

libbaremetal_la_LDFLAGS = $(AM_LDFLAGS) $(LTLDFLAGS)
libbaremetal_la_CPPFLAGS = $(AM_CPPFLAGS) -I $(OPTIMSOC_NEWLIB)/optimsoc-baremetal/include -Wall

baremetalincludedir = $(includedir)/baremetal
baremetalinclude_HEADERS = include/optimsoc-baremetal.h

scriptdir = $(datadir)/build/baremetal
script_DATA = share/create_linker_script.sh \
              share/link.ld.in \
              share/Makefile.inc \
              share/create_pgas_binary.sh \
              share/check_program_size.sh

# install the scripts with executable permissions
install-data-hook:
	chmod +x $(DESTDIR)$(scriptdir)/create_linker_script.sh
	chmod +x $(DESTDIR)$(scriptdir)/create_pgas_binary.sh
	chmod +x $(DESTDIR)$(scriptdir)/check_program_size.sh

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
pkgconfig_DATA = share/optimsoc-baremetal.pc
EXTRA_DIST = share/optimsoc-baremetal.pc.in
CLEANFILES = share/optimsoc-baremetal.pc

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

