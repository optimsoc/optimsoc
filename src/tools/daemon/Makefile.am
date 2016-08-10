LDADD = ../../libopensocdebug.la

bin_PROGRAMS = opensocdebugd

opensocdebugd_LDFLAGS = $(AM_LDFLAGS)
opensocdebugd_CFLAGS = $(AM_CFLAGS) -I$(top_srcdir)/src/include
opensocdebugd_SOURCES = daemon.c \
	handler.c
