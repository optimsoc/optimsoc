# Copyright (c) 2013 by the author(s)
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

lib_LTLIBRARIES = liboptimsocsysc.la

AM_CPPFLAGS = $(SYSTEMC_CFLAGS)
AM_LDFLAGS = $(SYSTEMC_LIBS)

liboptimsocsysc_la_SOURCES =\
    debug/DebugConnector.cpp \
    debug/VerilatedDebugConnector.cpp \
    debug/VerilatedSTM.cpp \
    debug/TracePacket.cpp

liboptimsocsysc_la_LDFLAGS = $(AM_LDFLAGS) $(LTLDFLAGS)
liboptimsocsysc_la_CPPFLAGS = $(AM_CPPFLAGS)
liboptimsocsysc_debugincdir = $(includedir)/debug
liboptimsocsysc_debuginc_HEADERS = debug/VerilatedDebugConnector.h \
	debug/DebugModule.h \
	debug/DebugConnector.h \
	debug/TracePacket.h \
	debug/VerilatedSTM.h

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
pkgconfig_DATA = share/optimsoc-sysc.pc
EXTRA_DIST = share/optimsoc-sysc.pc.in
CLEANFILES = share/optimsoc-sysc.pc