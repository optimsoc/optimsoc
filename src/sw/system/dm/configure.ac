AC_PREREQ(2.60)
AC_INIT([optimsoc-dm-libbaremetal],
        [0.1],
        [stefan.wallentowitz@tum.de],
        [optimsoc-dm-libbaremetal],
        [http://www.optimsoc.org])

AC_CONFIG_SRCDIR([src/libbaremetal/critical.c])
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

AM_PROG_AS

AC_PROG_CC_C99
#AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE
AC_CONFIG_MACRO_DIR([m4])
AM_SILENT_RULES([yes])
LT_INIT([disable-shared pic-only])
AC_PREFIX_DEFAULT([/usr/lib])

AC_ARG_WITH([optimsoc-newlib],
    [AS_HELP_STRING([--with-optimsoc-newlib],
                    [location of the optimsoc newlib])],
     [OPTIMSOC_NEWLIB="$withval"],
     [AS_IF([test "x$OPTIMSOC_NEWLIB" != "x"],
      [],
      [AC_MSG_FAILURE([Either set OPTIMSOC_NEWLIB or point to the newlib installation with --with-optimsoc-newlib])])
     ])

AC_SUBST(OPTIMSOC_NEWLIB)

AM_CONDITIONAL([OR32OBJDUMPSET],[test "x$OR32OBJDUMP" != "x"])
AM_CONDITIONAL([OR32OBJCOPYSET],[test "x$OR32OBJCOPY" != "x"])
AC_SUBST(OR32OBJDUMP)
AC_SUBST(OR32OBJCOPY)

AC_CONFIG_FILES([
        Makefile
        src/Makefile
        src/libbaremetal/Makefile
        src/libruntime/Makefile
        src/libmcapi/Makefile
])

AC_OUTPUT
