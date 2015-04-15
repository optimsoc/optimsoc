# Copyright (c) 2015 by the author(s)
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

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory

SUBDIRS = src

scriptbindir = $(bindir)
scriptbin_DATA = share/gzll-image-create

install-data-hook:
	chmod +x $(DESTDIR)$(scriptbindir)/gzll-image-create

scriptdir = $(datadir)/build/gzll/
script_DATA = share/Makefile-images.inc \
	share/gzll-image.ld

gzllincludedir = $(includedir)/gzll-kernel/
gzllinclude_HEADERS = include/gzll-syscall.h \
	include/gzll-apps.h

# prepare pkg-config file
share/%.pc: share/%.pc.in Makefile
	$(AM_V_GEN)$(MKDIR_P) share; \
	sed \
	-e 's,@VERSION\@,$(VERSION),g' \
	-e 's,@prefix\@,$(prefix),g' \
	-e 's,@datadir\@,$(datadir),g' \
	-e 's,@libdir\@,$(libdir),g' \
	-e 's,@includedir\@,$(includedir),g' \
	< $< > $@ || rm $@

pkgconfigdir = $(datadir)/pkgconfig
pkgconfig_DATA = share/gzll-kernel.pc

EXTRA_DIST = share/gzll-kernel.pc.in
CLEANFILES = share/gzll-kernel.pc
