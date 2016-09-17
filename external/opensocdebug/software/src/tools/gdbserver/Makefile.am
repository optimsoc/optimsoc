LDADD = ../../libopensocdebug.la

bin_PROGRAMS = osd-gdbserver

osd_gdbserver_LDFLAGS = $(AM_LDFLAGS) $(READLINE_LIBS)
osd_gdbserver_CFLAGS = $(AM_CFLAGS) -I$(top_srcdir)/src/include
osd_gdbserver_SOURCES = gdbserver.c
