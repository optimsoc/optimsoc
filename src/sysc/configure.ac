# Copyright (c) 2012-2013 by the author(s)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# Author(s):
#   Stefan Wallentowitz <stefan.wallentowitz@tum.de>

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

PKG_CHECK_MODULES([SYSTEMC], [systemc], , )
AC_SUBST([SYSTEMC_CFLAGS])
AC_SUBST([SYSTEMC_LIBS])

AC_CONFIG_FILES([
        Makefile
        src/Makefile
])

AC_OUTPUT
