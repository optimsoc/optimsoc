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
    dma.c \
    mp_simple.c \
    uart.c \
    util.c

libbaremetal_la_LDFLAGS = $(AM_LDFLAGS) $(LTLDFLAGS)
libbaremetal_la_CPPFLAGS = $(AM_CPPFLAGS) -Wall

scriptdir = $(datadir)/build/baremetal/
script_DATA = share/Makefile.inc \
              share/create_pgas_binary.sh

baremetalincludedir = $(includedir)/baremetal
baremetalinclude_HEADERS = include/optimsoc-baremetal.h

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

if OR1KOBJDUMPSET
MYOR1KOBJDUMP = $(OR1KOBJDUMP)
else
MYOR1KOBJDUMP = $(subst gcc,objdump,$(wordlist 1,1,$(CC)))
endif

if OR1KOBJCOPYSET
MYOR1KOBJCOPY = $(OR1KOBJCOPY)
else
MYOR1KOBJCOPY = $(subst gcc,objcopy,$(wordlist 1,1,$(CC)))
endif

share/Makefile.inc: share/Makefile.inc.in Makefile
	$(AM_V_GEN)$(MKDIR_P) share; \
	$(SED) -e 's,@OR1K-CC@,$(CC),g' -e 's,@OR1K-OBJDUMP@,$(MYOR1KOBJDUMP),g' \
	-e 's,@OR1K-OBJCOPY@,$(MYOR1KOBJCOPY),g' < $< > $@ || rm $@

CLEANFILES += share/Makefile.inc

share/create_pgas_binary.sh: share/create_pgas_binary.sh.in Makefile 
	$(AM_V_GEN)$(MKDIR_P) share; \
	$(SED) -e 's,@OR1K-CC@,$(CC),g' -e 's,@OR1K-OBJDUMP@,$(MYOR1KOBJDUMP),g' \
	-e 's,@OR1K-OBJCOPY@,$(MYOR1KOBJCOPY),g' < $< > $@ || rm $@

CLEANFILES += share/create_pgas_binary.sh
