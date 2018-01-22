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

cmake_minimum_required(VERSION 2.8.0)

project(optimsocgui)
set(VERSION_MAJOR "0")
set(VERSION_MINOR "2")
set(VERSION_PATCH "0")
set(VERSION_SUFFIX "pre")
set(PROJECT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_SUFFIX})

set(CMAKE_VERBOSE_MAKEFILE OFF)

# include our own cmake module files
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/)

# check for Qt
set(QT_MT_REQUIRED TRUE)
set(QT_MIN_VERSION "4.8.0")
find_package(Qt4 REQUIRED QtCore QtGui QtXml QtXmlPatterns QtWebkit)

include(${QT_USE_FILE})

# check for liboptimsochost
find_package(Liboptimsochost REQUIRED)

# check for libqwt
find_package(Libqwt REQUIRED)

# check for libxml2
find_package(LibXml2 REQUIRED)

# check for libxslt
find_package(LibXslt REQUIRED)

# Debug mode
option(DEFINE_DEBUG
  "Build the project using debugging code"
  ON)
if(DEFINE_DEBUG)
  add_definitions(-DDEBUG)
  set(CMAKE_BUILD_TYPE Debug)
else(DEFINE_DEBUG)
  set(CMAKE_BUILD_TYPE Release)
endif(DEFINE_DEBUG)
message(STATUS "Build type is " ${CMAKE_BUILD_TYPE})

# Doxygen API documentation
option(BUILD_APIDOCS "Use Doxygen to create the HTML based API documentation" ON)
if(BUILD_APIDOCS)
  find_package(Doxygen)
  if (NOT DOXYGEN_FOUND)
    message(FATAL_ERROR
      "Doxygen is needed to build the API documentation. Please install it.")
  endif()

  configure_file(doc/doxygen.cfg.in
                 ${PROJECT_BINARY_DIR}/doxygen.cfg @ONLY IMMEDIATE)

  add_custom_target(apidoc
                    COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/doxygen.cfg
                    SOURCES ${PROJECT_BINARY_DIR}/doxygen.cfg)
endif()

# include our main sources here
add_subdirectory(src)

message(STATUS "OpTiMSoC GUI ${PROJECT_VERSION} will be built to install into ${CMAKE_INSTALL_PREFIX}")
