AC_PREREQ(2.60)
AC_INIT([opensocdebug],
        [2016.1],
        [stefan@wallentowitz.de],
        [opensocdebug],
        [http://www.opensocdebug.org])
AC_CONFIG_SRCDIR([src/context.c])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([check-news foreign 1.11 -Wall -Wno-portability silent-rules tar-pax no-dist-gzip dist-xz subdir-objects])

# Library versioning
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
# http://sourceware.org/autobook/autobook/autobook_91.html
OSD_CURRENT=2016
OSD_REVISION=1
OSD_AGE=0
LTLDFLAGS="-version-info ${OSD_CURRENT}:${OSD_REVISION}:${OSD_AGE}"
AC_SUBST(LTLDFLAGS)

AC_PROG_CC_STDC
AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE
AC_CONFIG_MACRO_DIR([m4])
AM_SILENT_RULES([yes])
LT_INIT([disable-static pic-only])
AC_PREFIX_DEFAULT([/usr])

AC_PROG_SED
AC_PROG_MKDIR_P

# check for GLIP
PKG_CHECK_MODULES([libglip], [libglip >= 0.1])
AC_SUBST([libglip_CFLAGS])
AC_SUBST([libglip_LIBS])

# check for libreadline
AC_CHECK_LIB([readline], [readline], [
    READLINE_LIBS=-lreadline
    AC_DEFINE([HAVE_LIBREADLINE], [1], [Define to 1 if you have the `readline' library (-lreadline).])
])
AC_SUBST(READLINE_LIBS)

AC_CHECK_HEADER([readline/readline.h],,AC_MSG_ERROR([Unable to find readline.h. You may need to install the readline development package.]))

# check for libelf
AC_CHECK_LIB([elf], [elf_version], [
    LIBELF_LIBS=-lelf
    AC_DEFINE([HAVE_LIBELF], [1], [Define to 1 if you have the `elfutils' library (-lelf).])
])
AC_SUBST(LIBELF_LIBS)

AC_CHECK_HEADER([elfutils/version.h],,AC_MSG_ERROR([Unable to find elfutils/version.h. You may need to install the libelf-dev development package.]))

AC_ARG_ENABLE([python-bindings],
    AS_HELP_STRING([--enable-python-bindings], [enable python bindings (default n)]),
    [enable_python=$enableval],
    [enable_python=no])

AM_CONDITIONAL([HAVE_SWIG], [test "$SWIG"])
AS_IF([test "x$enable_python" = "xyes"], [
    AC_CHECK_PROG([SWIG], [swig], yes)
    AS_IF([test "x$SWIG" != "xyes"],[AC_MSG_ERROR([Unable to find swig. Needed for python bindings.])])
    AM_PATH_PYTHON()
    PYTHON_LIBS=`python-config --libs`
    PYTHON_INCLUDES=`python-config --includes`
    AC_SUBST(PYTHON_LIBS)
    AC_SUBST(PYTHON_INCLUDES)
    AC_DEFINE(ENABLE_PYTHON, [1], [Define to use python bindings])
])
AM_CONDITIONAL([ENABLE_PYTHON], [test "x$enable_python" != "xno"])

AM_CFLAGS="-Wall \
    -Wmissing-declarations -Wmissing-prototypes \
    -Wnested-externs -Wpointer-arith \
    -Wsign-compare -Wchar-subscripts \
    -Wstrict-prototypes -Wshadow \
    -Wformat-security -Wtype-limits \
    -fvisibility=hidden \
    -ffunction-sections \
    -fdata-sections \
    -pthread \
    ${libglip_CFLAGS}"
AC_SUBST([AM_CFLAGS])

AM_LDFLAGS="-Wl,--gc-sections \
    -Wl,--as-needed \
    -pthread \
    -lrt \
    ${libglip_LIBS}"
AC_SUBST(AM_LDFLAGS)

AC_CONFIG_HEADERS(config.h)
AC_CONFIG_FILES([
        Makefile
        opensocdebug.pc
        src/Makefile
        src/tools/Makefile
        src/tools/daemon/Makefile
        src/tools/osd_term/Makefile
        src/tools/cli/Makefile
        src/tools/gdbserver/Makefile
        src/bindings/Makefile
        src/bindings/python/Makefile
        src/bindings/python/setup.py
])

AC_OUTPUT
AC_MSG_RESULT([
        $PACKAGE $VERSION
        =====

        prefix:                 ${prefix}
        libdir:                 ${libdir}
        includedir:             ${includedir}
        datarootdir:            ${datarootdir}
        datadir:                ${datadir}

        compiler:               ${CC}
        cflags:                 ${CFLAGS}
        ldflags:                ${LDFLAGS}

        python bindings:        ${enable_python}
])
