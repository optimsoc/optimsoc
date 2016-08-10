SUBDIRS = . bindings tools

pkginclude_HEADERS = include/opensocdebug.h
lib_LTLIBRARIES = libopensocdebug.la

libopensocdebug_la_SOURCES = context.c \
	connection.c \
	connection-standalone.c \
	connection-daemon.c \
	system-info.c \
	regaccess.c \
	control.c \
	memory.c \
	trace.c

libopensocdebug_la_LDFLAGS = $(AM_LDFLAGS) $(LTLDFLAGS) $(LIBELF_LIBS)
libopensocdebug_la_CPPFLAGS = $(AM_CPPFLAGS) -include $(top_builddir)/config.h
