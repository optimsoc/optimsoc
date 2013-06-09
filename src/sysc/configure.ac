AC_PREREQ(2.60)
AC_INIT([optimsoc-sysc],
        [0.1],
        [stefan.wallentowitz@tum.de],
        [optimsoc-sysc],
        [http://www.optimsoc.org])

AC_CONFIG_SRCDIR([src/debug/DebugConnector.cpp])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([check-news foreign 1.11 -Wall -Wno-portability silent-rules tar-pax no-dist-gzip dist-xz subdir-objects])

# Library versioning
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
# http://sourceware.org/autobook/autobook/autobook_91.html
LIBOPTIMSOCSYSC_CURRENT=1
LIBOPTIMSOCSYSC_REVISION=0
LIBOPTIMSOCSYSC_AGE=0
LTLDFLAGS="-version-info ${LIBOPTIMSOCSYSC_CURRENT}:${LIBOPTIMSOCSYSC_REVISION}:${LIBOPTIMSOCSYSC_AGE}"
AC_SUBST(LTLDFLAGS)

AC_PROG_CXX
AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE
AC_CONFIG_MACRO_DIR([m4])
AM_SILENT_RULES([yes])
LT_INIT([pic-only])
AC_PREFIX_DEFAULT([/usr])

AC_PROG_SED
AC_PROG_MKDIR_P

AC_ARG_WITH([systemc-inc-dir],
    [AS_HELP_STRING([--with-systemc-inc-dir],
                    [location of the systemc include directory])],
     [SYSTEMC_CPPFLAGS="-I$withval"],
     [AC_MSG_FAILURE([You need to set the SystemC include dir])])

AC_ARG_WITH([systemc-lib-dir],
    [AS_HELP_STRING([--with-systemc-lib-dir],
                    [location of the systemc library directory])],
     [SYSTEMC_LDFLAGS="-L$withval -lsystemc"],
     [AC_MSG_FAILURE([You need to set the SystemC library dir])])

AC_SUBST([SYSTEMC_CPPFLAGS])
AC_SUBST([SYSTEMC_LDFLAGS])

AC_CONFIG_FILES([
        Makefile
        src/Makefile
])

AC_OUTPUT
