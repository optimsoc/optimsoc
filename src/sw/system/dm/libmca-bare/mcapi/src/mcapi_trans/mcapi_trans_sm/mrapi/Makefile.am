## Makefile.am -- Process this file with automake to produce Makefile.in


lib_LTLIBRARIES = libmcapi.la

libmcapi_la_SOURCES = $(top_srcdir)/mcapi/src/mcapi.c  ../transport_sm.c transport_sm_mrapi.c

MRAPI_CFLAGS = -I$(top_srcdir)/mrapi/src/mrapi_impl/sysvr4 -I$(top_srcdir)/mrapi/src/mrapi_impl -I$(top_srcdir)/mrapi/include

libmcapi_la_CFLAGS =  $(MRAPI_CFLAGS) 
libmcapi_la_LDFLAGS = -no-undefined $(MRAPI_LDFLAGS)

# include mrapi.h, mcapi.h mrapi_impl_spec.h mcapi_impl_spec.h and mca_config.h
INCLUDES =  -I$(top_srcdir)/mcapi/include -I$(top_srcdir)/common  -I$(top_srcdir)/mcapi/src/mcapi_trans/mcapi_trans_sm  -I$(top_srcdir)/ $(MRAPI_CFLAGS)


include_HEADERS = $(top_srcdir)/mcapi/include/mcapi.h $(top_srcdir)/mca_config.h $(top_srcdir)/mca_impl_spec.h $(top_srcdir)/mcapi/src/mcapi_trans/mcapi_trans_sm/mcapi_impl_spec.h 

CLEANFILES = *.gcov *.gcda *.gcno core.*

LINK = $(LIBTOOL) --tag=CC --mode=link $(CC) -o $@

install_libs = $(patsubst %.a,%,$(lib_LTLIBRARIES)) 
