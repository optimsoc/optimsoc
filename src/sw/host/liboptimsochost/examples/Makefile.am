LDADD = ../src/liboptimsochost.la

optimsoc_cli_LDFLAGS = $(AM_LDFLAGS) $(READLINE_LIBS)
optimsoc_cli_CFLAGS = $(AM_CFLAGS) -I$(top_srcdir)/src
optimsoc_cli_SOURCES = optimsoc_cli.c

optimsoc_reset_LDFLAGS = $(AM_LDFLAGS) $(READLINE_LIBS)
optimsoc_reset_CFLAGS = $(AM_CFLAGS) -I$(top_srcdir)/src
optimsoc_reset_SOURCES = optimsoc_reset.c

noinst_PROGRAMS = optimsoc_cli optimsoc_reset

