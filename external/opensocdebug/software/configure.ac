# Open SoC Debug version information
# Update this on every release. For API changes also update the library
# version below!
m4_define([OSD_VERSION_MAJOR_], [0])
m4_define([OSD_VERSION_MINOR_], [1])
m4_define([OSD_VERSION_MICRO_], [0])
m4_define([OSD_VERSION_SUFFIX_], [-dev])

AC_PREREQ(2.60)
AC_INIT([osd],
        [OSD_VERSION_MAJOR_[.]OSD_VERSION_MINOR_[.]OSD_VERSION_MICRO_[]OSD_VERSION_SUFFIX_],
        [opensocdebug@lists.librecores.org],
        [osd],
        [http://www.opensocdebug.org])

# Library versioning
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
# http://sourceware.org/autobook/autobook/autobook_91.html
OSD_CURRENT=0
OSD_REVISION=1
OSD_AGE=0
LTLDFLAGS="-version-info ${OSD_CURRENT}:${OSD_REVISION}:${OSD_AGE}"
AC_SUBST(LTLDFLAGS)


# define macros in config.h with the version information
AC_DEFINE([OSD_VERSION_MAJOR], [OSD_VERSION_MAJOR_], "OSD major version")
AC_DEFINE([OSD_VERSION_MINOR], [OSD_VERSION_MINOR_], "OSD minor version")
AC_DEFINE([OSD_VERSION_MICRO], [OSD_VERSION_MICRO_], "OSD micro version")
AC_DEFINE([OSD_VERSION_SUFFIX], ["OSD_VERSION_SUFFIX_"], "OSD version suffix")


AC_CONFIG_SRCDIR([src/libosd/include/osd/osd.h])
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
AC_PROG_MKDIR_P

# check unit testing framework
PKG_CHECK_MODULES([CHECK], [check >= 0.9.10])

# test memory checking with valgrind
# All other Valgrind tools show too many false positives to be useful
AX_VALGRIND_DFLT([memcheck], [on])
AX_VALGRIND_DFLT([sgcheck], [off])
AX_VALGRIND_DFLT([helgrind], [off])
AX_VALGRIND_DFLT([drd], [off])
AX_VALGRIND_CHECK

# test coverage with gcov and reporting with lcov
AX_CODE_COVERAGE

# Address Sanitizer
AC_MSG_CHECKING([whether to enable ASan])
AC_ARG_ENABLE([asan],
    [AS_HELP_STRING([--enable-asan], [Build with GCC Address Sanitizer instrumentation])],
    [],
    [enable_asan=no])
# Valgrind and ASan are mutually exclusive, as they use the same preload
# mechanism. Require the user to choose one.
AS_IF([test "x$enable_asan" = "xyes" -a "$VALGRIND_ENABLED" = "yes"],
   [AC_MSG_ERROR([--enable-asan requires --disable-valgrind.])]
)
AM_CONDITIONAL([ENABLE_ASAN], [test "x$enable_asan" = "xyes"])
AC_SUBST([ENABLE_ASAN])
AM_COND_IF([ENABLE_ASAN], [
   AC_MSG_RESULT([yes])
   CFLAGS="${CFLAGS} -fsanitize=address"
], [
   AC_MSG_RESULT([no])
])

# ZeroMQ and high-level C wrapper (CZMQ)
PKG_CHECK_MODULES([libzmq], [libzmq >= 4.1])
PKG_CHECK_MODULES([libczmq], [libczmq >= 3.0])

# libelf
PKG_CHECK_MODULES([libelf], [libelf])

# glip (device connectivity)
AC_ARG_WITH([glip],
    AS_HELP_STRING([--without-glip], [Ignore presence of glip and disable it]))

AS_IF([test "x$with_glip" != "xno"],
      [PKG_CHECK_MODULES([libglip], [libglip], [have_glip=yes], [have_glip=no])],
      [have_glip=no])

AS_IF([test "x$have_glip" = "xyes"],
      [AC_DEFINE(USE_GLIP, [1], [Make use of GLIP.])],
      [AS_IF([test "x$with_glip" = "xyes"],
             [AC_MSG_ERROR([glip requested but not found])
      ])
])
AM_CONDITIONAL([USE_GLIP], [test "x$have_glip" = "xyes"])

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
    AC_DEFINE(DEBUG, [1], [Produce a debug build])
])
AM_CONDITIONAL([DEBUG_BUILD], [test "x$enable_debug" = "xyes"])

# documentation
AC_ARG_ENABLE([docs],
    AS_HELP_STRING([--enable-docs], [build documentation @<:@default=disabled@:>@]),
    [],
    [enable_docs=no])
AS_IF([test "x$enable_docs" = "xyes"], [
    AC_CHECK_PROG([DOXYGEN], [doxygen], [doxygen])
    AS_IF([ test -z "$DOXYGEN" ], [
        AC_MSG_ERROR([doxygen is required to build the documentation.])
    ])
])
AM_CONDITIONAL([BUILD_DOCS], [test "x$enable_docs" = "xyes"])

AM_CFLAGS="-Wall \
    -Wmissing-declarations -Wmissing-prototypes \
    -Wnested-externs -Wpointer-arith \
    -Wsign-compare -Wchar-subscripts \
    -Wstrict-prototypes -Wshadow \
    -Wformat-security -Wtype-limits \
    -fno-omit-frame-pointer \
    -fstack-protector \
    -ffunction-sections \
    -fdata-sections \
    -pthread \
    $libczmq_CFLAGS \
    $libelf_CFLAGS \
    $CODE_COVERAGE_CFLAGS"
AC_SUBST([AM_CFLAGS])

AM_LDFLAGS="-Wl,--gc-sections \
    -Wl,--as-needed \
    -pthread \
    -lrt \
    $libczmq_LIBS \
    $libelf_LIBS \
    $CODE_COVERAGE_LIBS"
AC_SUBST(AM_LDFLAGS)

AC_CONFIG_HEADERS(config.h)
AC_CONFIG_FILES([
        Makefile
        src/Makefile
        src/libosd/Makefile
        src/tools/Makefile
        src/tools/osd-host-controller/Makefile
        src/tools/osd-device-gateway/Makefile
        src/tools/osd-target-run/Makefile
        tests/Makefile
        tests/unit/Makefile
        doc/Makefile
])

AC_OUTPUT
AC_MSG_RESULT([
Configuration summary for $PACKAGE $VERSION

PATHS
  prefix:                 ${prefix}
  libdir:                 ${libdir}
  includedir:             ${includedir}
  datarootdir:            ${datarootdir}
  datadir:                ${datadir}

COMPILER OPTIONS
  compiler:               ${CC}
  cflags:                 ${CFLAGS}
  ldflags:                ${LDFLAGS}])

AC_MSG_RESULT([
You can now run 'make' to start the build process.
])
