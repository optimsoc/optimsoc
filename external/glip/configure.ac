dnl Process this file with autoconf to produce a configure script.

dnl GLIP version information
dnl Update this on every release. For API changes also update the library
dnl version below!
m4_define([GL_VERSION_MAJOR], [0])
m4_define([GL_VERSION_MINOR], [1])
m4_define([GL_VERSION_MICRO], [0])
m4_define([GL_VERSION_SUFFIX], [-dev])

AC_PREREQ(2.60)
AC_INIT([glip],
        [GL_VERSION_MAJOR[.]GL_VERSION_MINOR[.]GL_VERSION_MICRO[]GL_VERSION_SUFFIX],
        [optimsoc@lis.ei.tum.de],
        [glip],
        [http://glip.io])

dnl Library versioning
dnl http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
dnl http://sourceware.org/autobook/autobook/autobook_91.html
GLIP_CURRENT=0
GLIP_REVISION=0
GLIP_AGE=0
LTLDFLAGS="-version-info ${GLIP_CURRENT}:${GLIP_REVISION}:${GLIP_AGE}"
AC_SUBST(LTLDFLAGS)

dnl define macros in config.h with the version information
AC_DEFINE([GLIP_VERSION_MAJOR], [GL_VERSION_MAJOR], "GLIP major version")
AC_DEFINE([GLIP_VERSION_MINOR], [GL_VERSION_MINOR], "GLIP minor version")
AC_DEFINE([GLIP_VERSION_MICRO], [GL_VERSION_MICRO], "GLIP micro version")
AC_DEFINE([GLIP_VERSION_SUFFIX], ["GL_VERSION_SUFFIX"], "GLIP version suffix")

AC_CONFIG_SRCDIR([src/glip.c])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([check-news foreign 1.11 -Wall -Wno-portability silent-rules \
                  tar-pax no-dist-gzip dist-xz subdir-objects])

AC_PROG_CC_STDC
AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE
AC_CONFIG_MACRO_DIR([m4])
AM_SILENT_RULES([yes])
LT_INIT([disable-static pic-only])
AC_PREFIX_DEFAULT([/usr])

AC_PROG_SED

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

dnl ----- BACKENDS -----
AC_DEFUN([GLIP_CONFIG_BACKEND], [
if test "x$enable_$1" = "xyes"; then
    AC_DEFINE(BACKEND_$2, [ENABLED_BACKEND($1)], [$2 backend.])
    AC_DEFINE(BACKEND_$2_ENABLED, 1, [$2 backend.])
else
    AC_DEFINE(BACKEND_$2, [DISABLED_BACKEND($1)], [$2 backend.])
fi
])

AC_ARG_ENABLE([uart],
    AS_HELP_STRING([--enable-uart], [enable UART backend @<:@default=enabled@:>@]),
    [],
    [enable_uart=yes])
AM_CONDITIONAL([BACKEND_UART], [test "x$enable_uart" = "xyes"])

GLIP_CONFIG_BACKEND(uart, UART)

AC_ARG_ENABLE([cypressfx2],
    AS_HELP_STRING([--enable-cypressfx2], [enable Cypress FX2 (USB 2.0) backend @<:@default=disabled@:>@]),
    [],
    [enable_cypressfx2=no])
AM_CONDITIONAL([BACKEND_CYPRESSFX2], [test "x$enable_cypressfx2" = "xyes"])

GLIP_CONFIG_BACKEND(cypressfx2, CYPRESSFX2)

AC_ARG_ENABLE([cypressfx3],
    AS_HELP_STRING([--enable-cypressfx3], [enable Cypress FX3 (USB 3.0) backend @<:@default=disabled@:>@]),
    [],
    [enable_cypressfx3=no])
AM_CONDITIONAL([BACKEND_CYPRESSFX3], [test "x$enable_cypressfx3" = "xyes"])

GLIP_CONFIG_BACKEND(cypressfx3, CYPRESSFX3)

AS_IF([test "x$enable_cypressfx2" = "xyes" -o "x$enable_cypressfx3" = "xyes"], [
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
])

AC_ARG_ENABLE([tcp],
    AS_HELP_STRING([--enable-tcp], [enable TCP backend @<:@default=enabled@:>@]),
    [],
    [enable_tcp=yes])
AM_CONDITIONAL([BACKEND_TCP], [test "x$enable_tcp" = "xyes"])

GLIP_CONFIG_BACKEND(tcp, TCP)


AC_ARG_ENABLE([jtag],
    AS_HELP_STRING([--enable-jtag], [enable JTAG backend @<:@default=disabled@:>@]),
    [],
    [enable_jtag=no])
AM_CONDITIONAL([BACKEND_JTAG], [test "x$enable_jtag" = "xyes"])

GLIP_CONFIG_BACKEND(jtag, JTAG)

dnl ----- COMPILER AND LINKER DEFAULT FLAGS -----
AM_CFLAGS="-Wall \
    -Wmissing-declarations -Wmissing-prototypes \
    -Wnested-externs -Wpointer-arith \
    -Wsign-compare -Wchar-subscripts \
    -Wstrict-prototypes -Wshadow \
    -Wformat-security -Wtype-limits \
    -fno-omit-frame-pointer \
    -fvisibility=hidden \
    -ffunction-sections \
    -fdata-sections \
    -pthread"
AC_SUBST([AM_CFLAGS])

AM_LDFLAGS="-Wl,--gc-sections \
            -Wl,--as-needed \
            -pthread \
            -lrt"
AC_SUBST(AM_LDFLAGS)


dnl ----- DOCUMENTATION -----
DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN(glip, doc/doxygen.cfg, apidoc)


dnl ---------------------

AC_CONFIG_HEADERS(config.h)

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    src/tools/Makefile
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

        backends:
        uart                    ${enable_uart}
        cypressfx2              ${enable_cypressfx2}
        cypressfx3              ${enable_cypressfx3}
        tcp                     ${enable_tcp}
        jtag                    ${enable_jtag}
])

