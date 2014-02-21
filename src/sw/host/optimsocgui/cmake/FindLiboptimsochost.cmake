#-*-cmake-*-
#
# Copyright (c) 2012-2014 by the author(s)
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
#   Philipp Wagner <philipp.wagner@tum.de>
#
#
# Once loaded this will define
#  LIBOPTIMSOCHOST_FOUND        - system has liboptimsochost
#  LIBOPTIMSOCHOST_INCLUDE_DIRS - include directory for liboptimsochost
#  LIBOPTIMSOCHOST_LIBRARIES    - libraries you need to link to
#  LIBOPTIMSOCHOST_DEFINITIONS  - required compiler switches for using this lib
#

find_package(PkgConfig)
pkg_check_modules(PC_LIBOPTIMSOCHOST QUIET liboptimsochost)
set(LIBOPTIMSOCHOST_DEFINITIONS ${PC_LIBOPTIMSOCHOST_CFLAGS_OTHER})

# HINTS gets searched before the system locations, PATHS after these
find_path(LIBOPTIMSOCHOST_INCLUDE_DIR liboptimsochost.h
  HINTS
    "${PC_LIBOPTIMSOCHOST_INCLUDEDIR}"
    "${PC_LIBOPTIMSOCHOST_INCLUDE_DIRS}"
  PATHS
    "$ENV{LIBOPTIMSOCHOST_LOCATION}/src"
    "$ENV{OPTIMSOC}/src/sw/host/liboptimsochost/src"
    "${PROJECT_SOURCE_DIR}/../liboptimsochost/src"
)

find_library(LIBOPTIMSOCHOST_LIBRARY
  NAMES optimsochost
  HINTS
    "${PC_LIBOPTIMSOCHOST_LIBDIR}"
    "${PC_LIBOPTIMSOCHOST_LIBRARY_DIRS}"
  PATHS
    "$ENV{LIBOPTIMSOCHOST_LOCATION}/src/.libs"
    "$ENV{OPTIMSOC}/src/sw/host/liboptimsochost/lib"
    "${PROJECT_SOURCE_DIR}/../liboptimsochost/lib"
  DOC "liboptimsochost library"
)


set(LIBOPTIMSOCHOST_LIBRARIES ${LIBOPTIMSOCHOST_LIBRARY})
set(LIBOPTIMSOCHOST_INCLUDE_DIRS ${LIBOPTIMSOCHOST_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBOPTIMSOCHOST DEFAULT_MSG
                                  LIBOPTIMSOCHOST_LIBRARY
                                  LIBOPTIMSOCHOST_INCLUDE_DIR)

mark_as_advanced(LIBOPTIMSOCHOST_LIBRARY LIBOPTIMSOCHOST_INCLUDE_DIR)
