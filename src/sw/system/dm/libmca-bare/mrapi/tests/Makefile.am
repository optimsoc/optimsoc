 ## Makefile.am -- Process this file with automake to produce Makefile.in

# after all programs are built, run this test script
TESTS = regress

CLEANFILES = core.* *.gcda *.gcno

check_PROGRAMS = test_sysvr4 sig1 node1 node2 node3 node4 barrier1 mutex1 mutex2 mutex3  sem1 sem2 sem3 shm shm1 shm2 shm3 errors rwl1 rwl2 rmem1 rmem2 rmem3 rmem4 rmem5 resources1 resources_get resource_attributes shared_memory_example rsrc_callbacks

EXTRA_DIST=regress


# the mrapi library
if BAREMETAL_FALSE
MRAPI_LIB = ../src/mrapi_impl/sysvr4/libmrapi.la
else
MRAPI_LIB = ../src/mrapi_impl/baremetal/powerpc/libmrapi.la
endif 
 
include_HEADERS = $(top_srcdir)/mrapi/include/mrapi.h barrier1.h
 
# include files & paths
INCLUDES = -I$(top_srcdir)/mrapi/include -I$(top_srcdir)/mrapi/src/mrapi_impl -I$(top_srcdir) -I$(top_srcdir)/common 


# programs (sources & libraries)
test_sysvr4_SOURCES = test_sysvr4.c

node1_SOURCES = node1.c 
node1_LDADD = $(MRAPI_LIB) 

sig1_SOURCES = sig1.c
sig1_LDADD = $(MRAPI_LIB)

node2_SOURCES = node2.c
node2_LDADD = $(MRAPI_LIB) -lpthread

node3_SOURCES = node3.c
node3_LDADD = $(MRAPI_LIB) -lpthread

node4_SOURCES = node4.c
node4_LDADD = $(MRAPI_LIB) -lpthread

barrier1_SOURCES = barrier1.c
barrier1_LDADD = $(MRAPI_LIB) -lpthread

mutex1_SOURCES = mutex1.c
mutex1_LDADD = $(MRAPI_LIB)

mutex2_SOURCES = mutex2.c
mutex2_LDADD = $(MRAPI_LIB) -lpthread

mutex3_SOURCES = mutex3.c
mutex3_LDADD = $(MRAPI_LIB) -lpthread

sem1_SOURCES = sem1.c
sem1_LDADD = $(MRAPI_LIB)

sem2_SOURCES = sem2.c
sem2_LDADD = $(MRAPI_LIB) -lpthread

sem3_SOURCES = sem3.c
sem3_LDADD = $(MRAPI_LIB) -lpthread

shm_SOURCES = shm.c
shm_LDADD = $(MRAPI_LIB)

shm1_SOURCES = shm1.c
shm1_LDADD = $(MRAPI_LIB) 

shm2_SOURCES = shm2.c
shm2_LDADD = $(MRAPI_LIB) -lpthread

shm3_SOURCES = shm3.c
shm3_LDADD = $(MRAPI_LIB) 

rmem1_SOURCES = rmem1.c
rmem1_LDADD = $(MRAPI_LIB)

rmem2_SOURCES = rmem2.c
rmem2_LDADD = $(MRAPI_LIB) -lpthread

rmem3_SOURCES = rmem3.c
rmem3_LDADD = $(MRAPI_LIB) -lpthread

rmem4_SOURCES = rmem4.c
rmem4_LDADD = $(MRAPI_LIB)

rmem5_SOURCES = rmem5.c
rmem5_LDADD = $(MRAPI_LIB) -lpthread

errors_SOURCES = errors.c
errors_LDADD = $(MRAPI_LIB)

rwl1_SOURCES = rwl1.c
rwl1_LDADD = $(MRAPI_LIB)

rwl2_SOURCES = rwl2.c
rwl2_LDADD = $(MRAPI_LIB) -lpthread

resources_get_SOURCES = resources_get.c
resources_get_LDADD = $(MRAPI_LIB)

resource_attributes_SOURCES = resource_attributes.c
resource_attributes_LDADD = $(MRAPI_LIB)

resources1_SOURCES = resources1.c
resources1_LDADD = $(MRAPI_LIB) -lpthread

shared_memory_example_SOURCES = shared_memory_example.c
shared_memory_example_LDADD = $(MRAPI_LIB) -lpthread

rsrc_callbacks_SOURCES = rsrc_callbacks.c
rsrc_callbacks_LDADD = $(MRAPI_LIB) -lpthread
