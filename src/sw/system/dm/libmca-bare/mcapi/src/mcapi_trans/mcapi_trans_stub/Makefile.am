## Makefile.am -- Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libmcapi.la

libmcapi_la_SOURCES = $(top_srcdir)/mcapi/src/mcapi.c  mcapi_trans_stub.c

noinst_HEADERS = mcapi_impl_spec.h

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/common -I$(top_srcdir)/mcapi/include $(MRAPI_CFLAGS) 


include_HEADERS = $(top_srcdir)/mcapi/include/mcapi.h $(top_srcdir)/mcapi/include/mcapi_trans.h mcapi_impl_spec.h 

LINK = $(LIBTOOL) --tag=CC --mode=link $(CC) -o $@

install_libs = $(patsubst %.a,%,$(lib_LTLIBRARIES)) 
