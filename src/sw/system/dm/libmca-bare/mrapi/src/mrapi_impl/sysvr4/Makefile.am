 ## Makefile.am -- Process this file with automake to produce Makefile.in

lib_LTLIBRARIES = libmrapi.la

INCLUDES = -I $(top_srcdir) -I$(top_srcdir)/common -I$(top_srcdir)/mrapi/include -I$(top_srcdir)/mrapi/src/mrapi_impl -I$(top_srcdir)/mrapi/src/mrapi_impl/sysvr4 


libmrapi_la_SOURCES = ../../mrapi.c ../mrapi_impl_spec.c sysvr4.c $(top_srcdir)/common/mca_utils.c 

install_libs = libmrapi

SYSVR4 = $(shell pwd)
