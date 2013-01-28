  ## Makefile.am -- Process this file with automake to produce Makefile.in
  
TESTS = regress

EXTRA_DIST = regress

CLEANFILES = core.* *.gcda *.gcno

check_PROGRAMS =  tc_echo_single_thread sig sig2 sig3 sig4 sig5 sig6 sig7 endpoints1 endpoints2 node1 node2  msg1 msg2 msg3 msg4 msg5 msg6 msg7 msg8 msg9 msg10 msg11 msg12 msg13 msg14 msg15 msg16 msg17  pkt1 pkt2 pkt3 pkt4 pkt5 pkt6 pkt7  scl1 scl2 scl3 mtapi_test3 multiMessage multiMessage_single_thread  

include_HEADERS = $(top_srcdir)/mcapi/include/mcapi.h

MRAPI_CFLAGS = -I$(top_srcdir)/mrapi/include 

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/common -I$(top_srcdir)/mcapi/include -I$(top_srcdir)/mcapi/src/mcapi_trans/mcapi_trans_sm $(MRAPI_CFLAGS) 

MCAPI_LTLIBS = ../src/mcapi_trans/mcapi_trans_sm/mrapi/libmcapi.la

if BAREMETAL
MRAPI_LTLIBS = ../../mrapi/src/mrapi_impl/baremetal/powerpc/libmrapi.la
else
MRAPI_LTLIBS = ../../mrapi/src/mrapi_impl/sysvr4/libmrapi.la 
endif

AM_CFLAGS = -DTIMEOUT=1000000

sig_SOURCES = sig.c signals.c
sig_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

sig2_SOURCES = sig2.c signals.c
sig2_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS) -lpthread

sig3_SOURCES = sig3.c signals.c
sig3_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS) -lpthread

sig4_SOURCES = sig4.c
sig4_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

sig5_SOURCES = sig5.c
sig5_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS) -lpthread

sig6_SOURCES = sig6.c
sig6_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS) 

sig7_SOURCES = sig7.c signals.c
sig7_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

endpoints1_SOURCES = endpoints1.c
endpoints1_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

endpoints2_SOURCES = endpoints2.c
endpoints2_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

node1_SOURCES = node1.c
node1_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

node2_SOURCES = node2.c
node2_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

msg1_SOURCES = msg1.c
msg1_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

msg2_SOURCES = msg2.c
msg2_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS) -lpthread

msg3_SOURCES = msg3.c
msg3_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS) -lpthread

msg4_SOURCES = msg4.c
msg4_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS) -lpthread

msg5_SOURCES = msg5.c
msg5_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS) -lpthread

msg6_SOURCES = msg6.c
msg6_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

msg7_SOURCES = msg7.c
msg7_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

msg8_SOURCES = msg8.c
msg8_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

msg9_SOURCES = msg9.c
msg9_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

msg10_SOURCES = msg10.c
msg10_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

msg11_SOURCES = msg11.c
msg11_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

msg12_SOURCES = msg12.c
msg12_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS) 

msg13_SOURCES = msg13.c
msg13_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

msg14_SOURCES = msg14.c
msg14_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

msg15_SOURCES = msg15.c
msg15_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

msg16_SOURCES = msg16.c
msg16_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

msg17_SOURCES = msg17.c
msg17_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

pkt1_SOURCES = pkt1.c
pkt1_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

pkt2_SOURCES = pkt2.c
pkt2_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

pkt3_SOURCES = pkt3.c
pkt3_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

pkt4_SOURCES = pkt4.c
pkt4_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

pkt5_SOURCES = pkt5.c
pkt5_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

pkt6_SOURCES = pkt6.c
pkt6_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS) -lpthread

pkt7_SOURCES = pkt7.c
pkt7_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)


scl1_SOURCES = scl1.c
scl1_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

scl2_SOURCES = scl2.c
scl2_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS) -lpthread
                                                                                                                                                   
scl3_SOURCES = scl3.c
scl3_LDADD = $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

mtapi_test3_SOURCES = mtapi_test3.c
mtapi_test3_LDADD =  $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS) -lpthread

multiMessage_SOURCES = multiMessage.c
multiMessage_LDADD =  $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS) -lpthread

multiMessage_single_thread_SOURCES = multiMessage_single_thread.c
multiMessage_single_thread_LDADD =  $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

tc_echo_single_thread_SOURCES = tc_echo_single_thread.c
tc_echo_single_thread_LDADD =  $(MCAPI_LTLIBS)  $(MRAPI_LTLIBS)

 
