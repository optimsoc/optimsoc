LDADD = ../liboptimsochost.la

bin_PROGRAMS = optimsoc_cli optimsoc_reset

optimsoc_cli_LDFLAGS = $(AM_LDFLAGS) $(READLINE_LIBS) $(PYTHON_LIBS)
optimsoc_cli_CFLAGS = $(AM_CFLAGS) $(PYTHON_INCLUDES) -I$(top_srcdir)/src
optimsoc_cli_SOURCES = optimsoc_cli.c
if USE_PYTHON 
    optimsoc_cli_SOURCES += optimsoc_cli_python.c
endif

optimsoc_reset_LDFLAGS = $(AM_LDFLAGS) $(READLINE_LIBS)
optimsoc_reset_CFLAGS = $(AM_CFLAGS) -I$(top_srcdir)/src
optimsoc_reset_SOURCES = optimsoc_reset.c
