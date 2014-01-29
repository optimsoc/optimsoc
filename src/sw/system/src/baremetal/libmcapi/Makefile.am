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
#   Philipp Wagner <philipp.wagner@tum.de>

libmcapi_LTLIBRARIES = libmcapi-simple.la libmcapi-simple-runtime.la
libmcapidir = $(libdir)/baremetal

libmcapi_simple_la_SOURCES = mcapi.c \
                             mcapi_trans_simple/base.c \
                             mcapi_trans_simple/channels.c \
                             mcapi_trans_simple/endpoint.c \
                             mcapi_trans_simple/messages.c
                             
libmcapi_simple_la_LDFLAGS = $(AM_LDFLAGS) $(LTLDFLAGS)
libmcapi_simple_la_CPPFLAGS = $(AM_CPPFLAGS) -Wall \
                              -I $(OPTIMSOC_NEWLIB)/optimsoc-baremetal/include \
                              -I $(top_srcdir)/src/baremetal/libbaremetal/include \
                              -I $(top_srcdir)/src/baremetal/libmcapi/include/

libmcapi_simple_runtime_la_SOURCES = mcapi.c \
                                     mcapi_trans_simple/base.c \
                                     mcapi_trans_simple/channels.c \
                                     mcapi_trans_simple/endpoint.c \
                                     mcapi_trans_simple/messages.c \
                                     control.c \
                                     endpoints.c \
                                     mcapi_syscall_mapper/mcapi_syscall_mapper.c
    
libmcapi_simple_runtime_la_LDFLAGS = $(AM_LDFLAGS) $(LTLDFLAGS)
libmcapi_simple_runtime_la_CPPFLAGS = $(AM_CPPFLAGS) -DRUNTIME -Wall\
                                      -I $(OPTIMSOC_NEWLIB)/optimsoc-baremetal/include \
                                      -I $(top_srcdir)/src/baremetal/libbaremetal/include \
                                      -I $(top_srcdir)/src/baremetal/libmcapi/include/ \
                                      -I $(top_srcdir)/src/baremetal/libruntime/include/ \
                                      -I $(top_srcdir)/src/baremetal/libruntime/

mcapiincludedir = $(includedir)/baremetal/mcapi
mcapiinclude_HEADERS = include/mcapi.h include/mca_config.h \
	include/mca_impl_spec.h include/mca.h include/mcapi_impl_spec.h \
	include/mcapi_trans.h include/mcapi_syscall_mapper.h

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
pkgconfig_DATA = share/optimsoc-baremetal-mcapi-simple.pc \
		 share/optimsoc-baremetal-mcapi-simple-runtime.pc
EXTRA_DIST = share/optimsoc-baremetal-mcapi-simple.pc.in \
             share/optimsoc-baremetal-mcapi-simple-runtime.pc.in
CLEANFILES = share/optimsoc-baremetal-mcapi-simple.pc \
             share/optimsoc-baremetal-mcapi-simple-runtime.pc

