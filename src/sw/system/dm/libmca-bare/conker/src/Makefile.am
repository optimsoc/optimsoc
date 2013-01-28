## Makefile.am -- Process this file with automake to produce Makefile.in

bin_PROGRAMS =


# turn off pthread and fork versions if we are building for baremetal (no OS) 
UTILS = $(top_srcdir)/conker/src/posix_utils.c
bin_PROGRAMS +=   \
	tc_example_pthread \
	tc_example_fork \
	tc_echo_pthread \
	tc_echo_fork \
	tc_overlap_pthread \
	tc_overlap_fork \
	tc_fury_pthread \
	tc_fury_fork \
	tc_lock_pthread \
	tc_lock_fork

# add mtapi if we have it
#if BUILD_MTAPI
bin_PROGRAMS += \
        tc_example_mtapi \
        tc_echo_mtapi \
        tc_overlap_mtapi \
        tc_fury_mtapi 
#endif


MRAPI_CFLAGS = -I$(top_srcdir)/mrapi/include -I$(top_srcdir)/mrapi/src/mrapi_impl -I$(top_srcdir)/mrapi/src/mrapi_impl/sysvr4
MCAPI_CFLAGS = -I$(top_srcdir)/mcapi/include -I$(top_srcdir)/mcapi/src/mcapi_trans/mcapi_trans_sm
MTAPI_CFLAGS = -I$(top_srcdir)/mtapi/include
MCAPI_LTLIBS = ../../mcapi/src/mcapi_trans/mcapi_trans_sm/mrapi/libmcapi.la

if BAREMETAL
MRAPI_LTLIBS = ../../mrapi/src/mrapi_impl/baremetal/powerpc/libmrapi.la
else
MRAPI_LTLIBS = ../../mrapi/src/mrapi_impl/sysvr4/libmrapi.la
endif

MTAPI_LTLIBS = ../../mtapi/src/libmtapi.la


# all non-baremetal programs need to link in pthread because the test harness uses it (even for forked versions)

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/common -I$(top_srcdir)/conker/include -I../ $(MRAPI_CFLAGS) $(MCAPI_CFLAGS) $(MTAPI_CFLAGS) 

common_SOURCES = $(top_srcdir)/common/mca_utils.c $(UTILS) $(top_srcdir)/conker/src/th_common.c

fork_SOURCES = $(top_srcdir)/conker/src/th_fork.c

pthread_SOURCES = $(top_srcdir)/conker/src/th_pthreads.c

mtapi_SOURCES = $(top_srcdir)/conker/src/th_mtapi.c

# pthread versions
tc_example_pthread_SOURCES =  $(top_srcdir)/conker/src/tc_example.c \
	$(pthread_SOURCES) \
	$(common_SOURCES)
tc_example_pthread_LDADD = -lpthread $(MCAPI_LTLIBS) $(MRAPI_LTLIBS)

tc_echo_pthread_SOURCES = $(top_srcdir)/conker/src/tc_echo.c \
	$(pthread_SOURCES) \
	$(common_SOURCES)
tc_echo_pthread_LDADD = -lpthread $(MCAPI_LTLIBS) $(MRAPI_LTLIBS)

tc_overlap_pthread_SOURCES = $(top_srcdir)/conker/src/tc_overlap.c  \
	$(pthread_SOURCES)\
	 $(common_SOURCES)
tc_overlap_pthread_LDADD = -lpthread $(MCAPI_LTLIBS) $(MRAPI_LTLIBS)

tc_fury_pthread_SOURCES = $(top_srcdir)/conker/src/tc_fury.c \
	$(pthread_SOURCES) \
	$(common_SOURCES)
tc_fury_pthread_LDADD = -lpthread $(MCAPI_LTLIBS) $(MRAPI_LTLIBS)

tc_lock_pthread_SOURCES =  $(top_srcdir)/conker/src/tc_lock.c \
	$(pthread_SOURCES) \
	$(common_SOURCES)
tc_lock_pthread_LDADD = -lpthread $(MCAPI_LTLIBS) $(MRAPI_LTLIBS)

# fork versions
tc_example_fork_SOURCES =  $(top_srcdir)/conker/src/tc_example.c \
	$(fork_SOURCES) \
	$(common_SOURCES)
tc_example_fork_LDADD = -lpthread $(MCAPI_LTLIBS) $(MRAPI_LTLIBS)

tc_echo_fork_SOURCES = $(top_srcdir)/conker/src/tc_echo.c \
	$(fork_SOURCES) \
	$(common_SOURCES)  
tc_echo_fork_LDADD = -lpthread $(MCAPI_LTLIBS) $(MRAPI_LTLIBS)

tc_overlap_fork_SOURCES = $(top_srcdir)/conker/src/tc_overlap.c  \
	$(fork_SOURCES)\
	 $(common_SOURCES)
tc_overlap_fork_LDADD = -lpthread $(MCAPI_LTLIBS) $(MRAPI_LTLIBS)

tc_fury_fork_SOURCES = $(top_srcdir)/conker/src/tc_fury.c \
	$(fork_SOURCES) \
	$(common_SOURCES)
tc_fury_fork_LDADD = -lpthread $(MCAPI_LTLIBS) $(MRAPI_LTLIBS)

tc_lock_fork_SOURCES =  $(top_srcdir)/conker/src/tc_lock.c \
	$(fork_SOURCES) \
	$(common_SOURCES)
tc_lock_fork_LDADD = -lpthread $(MCAPI_LTLIBS) $(MRAPI_LTLIBS)

# mtapi versions
tc_example_mtapi_SOURCES =  $(top_srcdir)/conker/src/tc_example.c \
	$(mtapi_SOURCES) \
	$(common_SOURCES)
tc_example_mtapi_LDADD = $(MTAPI_LTLIBS) $(MCAPI_LTLIBS) $(MRAPI_LTLIBS) -lpthread  

tc_echo_mtapi_SOURCES = $(top_srcdir)/conker/src/tc_echo.c \
	$(mtapi_SOURCES) \
	$(common_SOURCES)
tc_echo_mtapi_LDADD = $(MTAPI_LTLIBS) $(MCAPI_LTLIBS) $(MRAPI_LTLIBS) -lpthread 

tc_overlap_mtapi_SOURCES = $(top_srcdir)/conker/src/tc_overlap.c  \
	$(mtapi_SOURCES)\
	 $(common_SOURCES)
tc_overlap_mtapi_LDADD = $(MTAPI_LTLIBS) $(MCAPI_LTLIBS) $(MRAPI_LTLIBS) -lpthread 

tc_fury_mtapi_SOURCES = $(top_srcdir)/conker/src/tc_fury.c \
	$(mtapi_SOURCES) \
	$(common_SOURCES)
tc_fury_mtapi_LDADD = $(MTAPI_LTLIBS) $(MCAPI_LTLIBS) $(MRAPI_LTLIBS) -lpthread 



LINK = $(LIBTOOL) --tag=CC --mode=link $(CC) $(LDFLAGS) -o $@
