## AM_C_SETUP
##
## This performs a variety of checks related to C usage and sets various
## variables accordingly.  The idea is that you include the variable CSETUP
## when defining CXXFLAGS.
##
## If the --disable-opt option is set, we disable optimization.  Otherwise,
## it's on by default.
##
## If --enable-profile is set, PROFILE is set to a flag.
## 
## We also define, and substitute, the variable RFLAG, which is set to something
## that should act as an option to encode a path for library lookup.  You can use
## this in a config file, e.g. "@RFLAG@/foo/bar".
##
## AC_DEFUN([MAJOR],[MINOR])
##
## If major and minor are ommitted, no version checking is performed.  If included,
## GCC must have at least this version number in order to pass.
##
AC_DEFUN([AM_C_SETUP],
[

RFLAG="-Wl,-R"
AC_SUBST(RFLAG)


AC_ARG_ENABLE(opt,
[  --enable-opt    Enable optimization flag (-O3), disables debug],
 if [[[ x${enableval} != xno ]]] ; then
   OPT="-O3"
   AC_MSG_RESULT([Optimizing with -O3])	
 else
   OPT="-fkeep-inline-functions -fno-inline"
   AC_MSG_RESULT([Optimization disabled.])	
 fi
 ,
 OPT="-O3"
 AC_MSG_RESULT([Optimizing with -O3 by default.]),
)



PROFILE=
AC_ARG_ENABLE(profile,
[  --enable-profile    Enable profiling],
 PROFILE="-pg"
 AC_MSG_RESULT("Profiling with -pg")
)


coverage=
AC_ARG_ENABLE(coverage,
[  --enable-coverage    Enable coverage (using gcov)],
 COVERAGE="-fprofile-arcs -ftest-coverage"
 AC_MSG_RESULT("Coverage data collection using gcov enabled")
)

##
## Include this flag in CXXFLAGS.
##

CSETUP="$OPT $PROFILE $COVERAGE"

##
## Remember GCC major, minor, and micro versions. Sometimes
## micro is missing. In those cases, set micro to 0.
##
GCC_VERSION=`$CC -dumpversion`
## Note the use of quadrigraphs below to get autoconf to put $1 and $2
## into the awk script.  Refer to the autoconf manual for more information.
GCC_MAJOR=`echo $GCC_VERSION | awk -F. ' { print @S|@1; }'`
GCC_MINOR=`echo $GCC_VERSION | awk -F. ' { print @S|@2; }'`
AC_MSG_RESULT([GCC Version:  $GCC_MAJOR.$GCC_MINOR])

REQUIRED_MAJOR=$1
REQUIRED_MINOR=$2

## Do version checking.
AC_MSG_CHECKING([that gcc has the required version ($REQUIRED_MAJOR.$REQUIRED_MINOR)])
if [[ x$REQUIRED_MAJOR != x ]]; then
  if [[ $GCC_MAJOR -lt $REQUIRED_MAJOR ]]; then
    AC_MSG_ERROR([GCC must have a minimum major version of $REQUIRED_MAJOR.])
  fi
  if [[ x$REQUIRED_MINOR != x -a $GCC_MAJOR -eq $REQUIRED_MAJOR ]]; then
    if [[ $GCC_MINOR -lt $REQUIRED_MINOR ]]; then
      AC_MSG_ERROR([GCC must have a minimum minor version of $REQUIRED_MINOR.])
    fi
  fi
fi
AC_MSG_RESULT([okay])

AC_SUBST(GCC_VERSION)
AC_SUBST(GCC_MAJOR)
AC_SUBST(GCC_MINOR)

])
