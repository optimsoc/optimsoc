#-*-cmake-*-
#
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
#   Philipp Wagner <philipp.wagner@tum.de>
#
#
# Once loaded this will define
#  LIBOPTIMSOCHOST_FOUND        - system has liboptimsochost
#  LIBOPTIMSOCHOST_INCLUDE_DIR  - include directory for liboptimsochost
#  LIBOPTIMSOCHOST_LIBRARIES    - libraries you need to link to
#

set(LIBOPTIMSOCHOST_FOUND   "NO")

find_path(LIBOPTIMSOCHOST_INCLUDE_DIR liboptimsochost.h
  "$ENV{LIBOPTIMSOCHOST_LOCATION}/src/optimsochost"
  "$ENV{OPTIMSOC}/src/sw/host/liboptimsochost/src/optimsochost"
  "${PROJECT_SOURCE_DIR}/../liboptimsochost/src/optimsochost"
  /usr/include/optimsochost
)

find_library(LIBOPTIMSOCHOST_LIBRARY
  NAMES optimsochost
  PATHS
  "$ENV{LIBOPTIMSOCHOST_LOCATION}/src/.libs"
  "$ENV{OPTIMSOC}/src/sw/host/liboptimsochost/lib"
  "${PROJECT_SOURCE_DIR}/../liboptimsochost/lib"
  DOC "liboptimsochost library"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBOPTIMSOCHOST DEFAULT_MSG
                                  LIBOPTIMSOCHOST_LIBRARY
                                  LIBOPTIMSOCHOST_INCLUDE_DIR)

if(LIBOPTIMSOCHOST_FOUND)
  set(LIBOPTIMSOCHOST_LIBRARIES ${LIBOPTIMSOCHOST_LIBRARY})
endif(LIBOPTIMSOCHOST_FOUND)

mark_as_advanced(LIBOPTIMSOCHOST_LIBRARY LIBOPTIMSOCHOST_INCLUDE_DIR)
