LDADD = ../../libopensocdebug.la

bin_PROGRAMS = osd-cli

osd_cli_LDFLAGS = $(AM_LDFLAGS) $(READLINE_LIBS)
osd_cli_CFLAGS = $(AM_CFLAGS) -I$(top_srcdir)/src/include
osd_cli_SOURCES = cli.c \
	memory_test.c \
	terminal.c
