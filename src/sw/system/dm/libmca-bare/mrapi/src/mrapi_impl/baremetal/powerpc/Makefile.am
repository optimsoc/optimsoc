 ## Makefile.am -- Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libmrapi.la

INCLUDES = -I $(top_srcdir) -I$(top_srcdir)/common -I$(top_srcdir)/mrapi/include -I$(top_srcdir)/mrapi/src/mrapi_impl -I$(top_srcdir)/mrapi/src/mrapi_impl/baremetal/powerpc 

noinst_HEADERS = mrapi_powerpc_lock.h

libmrapi_la_SOURCES = mrapi_powerpc.c ../../../mrapi.c ../../mrapi_impl_spec.c $(top_srcdir)/common/mca_utils.c 

install_libs = libmrapi

