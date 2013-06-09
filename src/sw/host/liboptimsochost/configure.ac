AC_PREREQ(2.60)
AC_INIT([optimsochost],
        [0.1],
        [mail@philipp-wagner.com],
        [optimsochost],
        [http://www.lis.ei.tum.de/?id=optimsoc])
AC_CONFIG_SRCDIR([src/liboptimsochost.c])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([check-news foreign 1.11 -Wall -Wno-portability silent-rules tar-pax no-dist-gzip dist-xz subdir-objects])

# Library versioning
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
# http://sourceware.org/autobook/autobook/autobook_91.html
LIBOPTIMSOC_CURRENT=1
LIBOPTIMSOC_REVISION=0
LIBOPTIMSOC_AGE=0
LTLDFLAGS="-version-info ${LIBOPTIMSOC_CURRENT}:${LIBOPTIMSOC_REVISION}:${LIBOPTIMSOC_AGE}"
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

AC_ARG_ENABLE([logging],
    AS_HELP_STRING([--disable-logging], [disable system logging @<:@default=enabled@:>@]),
    [],
    [enable_logging=yes])
AS_IF([test "x$enable_logging" = "xyes"], [
    AC_DEFINE(LOGGING, [1], [System logging.])
])

AC_ARG_ENABLE([debug],
    AS_HELP_STRING([--enable-debug], [enable debug messages @<:@default=disabled@:>@]),
    [],
    [enable_debug=no])
AS_IF([test "x$enable_debug" = "xyes"], [
    AC_DEFINE(DEBUG, [1], [Debug messages.])
])

AC_ARG_ENABLE([examples],
    AS_HELP_STRING([--enable-examples], [build example applications (default n)])],
    [build_examples=$enableval],
    [build_examples=no])
AM_CONDITIONAL([BUILD_EXAMPLES], [test "x$build_examples" != "xno"])

# check for libusb 1.0
#
# The libusb function libusb_error_string() was introduced with libusb 1.0.9.
# To enable compilation with older libusb versions, we can use a fallback
# implementation if libusb is too old.
# XXX: Replace with a proper feature test instead of testing for the version
#      number.
PKG_CHECK_MODULES([libusb], [libusb-1.0 > 1.0.8],
[
    AC_DEFINE(LIBUSB_HAS_ERROR_STRING, [1], [Define to 1 if libusb has libusb_error_string() function.])
],
[
    PKG_CHECK_MODULES([libusb], [libusb-1.0 > 1.0.0])
])
AC_SUBST([libusb_CFLAGS])
AC_SUBST([libusb_LIBS])


# check for libreadline
AC_CHECK_LIB([readline], [readline], [
    READLINE_LIBS=-lreadline
    AC_DEFINE([HAVE_LIBREADLINE], [1], [Define to 1 if you have the `readline' library (-lreadline).])
])
AC_SUBST(READLINE_LIBS)


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
    ${libusb_CFLAGS}"
AC_SUBST([AM_CFLAGS])

AM_LDFLAGS="-Wl,--gc-sections \
    -Wl,--as-needed \
    -pthread \
    -lrt \
    ${libusb_LIBS}"
AC_SUBST(AM_LDFLAGS)

AC_CONFIG_HEADERS(config.h)
AC_CONFIG_FILES([
        Makefile
        src/Makefile
        examples/Makefile
        doc/Makefile
        doc/doxygen.cfg
])

AC_OUTPUT
AC_MSG_RESULT([
        $PACKAGE $VERSION
        =====

        prefix:                 ${prefix}
        sysconfdir:             ${sysconfdir}
        libdir:                 ${libdir}
        includedir:             ${includedir}

        compiler:               ${CC}
        cflags:                 ${CFLAGS}
        ldflags:                ${LDFLAGS}

        logging:                ${enable_logging}
        debug:                  ${enable_debug}
        build examples:         ${build_examples}
])
