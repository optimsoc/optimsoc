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

# add all *.cpp files here
SET(optimsocgui_SRCS
    aboutdialog.cpp
    connectionview.cpp
    executionchart.cpp
    executionchartcreators.cpp
    executionchartelements.cpp
    executionchartplots.cpp
    hexspinbox.cpp
    logwidget.cpp
    main.cpp
    mainwindow.cpp
    memory.cpp
    memoryinteractionwidget.cpp
    memorytablemodel.cpp
    optimsocsystem.cpp
    optimsocsystemelement.cpp
    optimsocsystemelementtreeitem.cpp
    optimsocsystemfactory.cpp
    optimsocsystemmodel.cpp
    plotspectrogram.cpp
    softwareexecutionview.cpp
    systeminterface.cpp
    systeminterfaceworker.cpp
    systemoverviewjsapi.cpp
    systemoverviewwidget.cpp
    systemview.cpp
    traceevents.cpp
    util.cpp

    tile/tile.cpp
    tile/computetile/computetile.cpp
    tile/externaltile/externaltile.cpp
    tile/memorytile/memorytile.cpp

    3rdparty/qcustomplot/qcustomplot.cpp
)

# add all *.ui files here
SET(optimsocgui_UIS
    aboutdialog.ui
    connectionview.ui
    executionchart.ui
    logwidget.ui
    mainwindow.ui
    memoryinteractionwidget.ui
    plotspectrogram.ui
    softwareexecutionview.ui
    systemview.ui
)

# add all files which contain classes using the QT_OBJECT macro here
SET(optimsocgui_MOC_HEADERS
    aboutdialog.h
    connectionview.h
    executionchartcreators.h
    executionchartelements.h
    executionchart.h
    executionchartplots.h
    hexspinbox.h
    logwidget.h
    mainwindow.h
    memory.h
    memoryinteractionwidget.h
    memorytablemodel.h
    optimsocsystemelement.h
    optimsocsystemelementtreeitem.h
    optimsocsystemfactory.h
    optimsocsystem.h
    optimsocsystemmodel.h
    plotspectrogram.h
    softwareexecutionview.h
    systeminterface.h
    systeminterfaceworker.h
    systemoverviewjsapi.h
    systemoverviewwidget.h
    systemview.h
    traceevents.h

    tile/tile.h
    tile/computetile/computetile.h
    tile/externaltile/externaltile.h
    tile/memorytile/memorytile.h

    3rdparty/qcustomplot/qcustomplot.h
)

SET(optimsocgui_QRCS
    optimsocgui.qrc
)

# Debug mode
OPTION(DEFINE_DEBUG
  "Build the project using debugging code"
  ON)
IF(DEFINE_DEBUG)
  ADD_DEFINITIONS(-DDEBUG)
  SET(CMAKE_BUILD_TYPE Debug)
  MESSAGE("Build type is " ${CMAKE_BUILD_TYPE})
ENDIF(DEFINE_DEBUG)


QT4_ADD_RESOURCES(optimsocgui_QRCS_SRCS ${optimsocgui_QRCS})

# generate ui_*.h files (inside CMAKE_CURRENT_BINARY_DIR)
QT4_WRAP_UI(optimsocgui_UIS_H ${optimsocgui_UIS})

include_directories(
  ${LIBOPTIMSOCHOST_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}

  ${CMAKE_CURRENT_SOURCE_DIR}/tile
  ${CMAKE_CURRENT_SOURCE_DIR}/tile/computetile
  ${CMAKE_CURRENT_SOURCE_DIR}/tile/externaltile
  ${CMAKE_CURRENT_SOURCE_DIR}/tile/memorytile

  ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/qcustomplot
)

# generate moc_*.cpp files
QT4_WRAP_CPP(optimsocgui_MOC_SRCS ${optimsocgui_MOC_HEADERS})

# generate executable
add_executable(optimsocgui
  ${optimsocgui_SRCS}
  ${optimsocgui_UIS_H}
  ${optimsocgui_MOC_SRCS}
  ${optimsocgui_QRCS_SRCS}
)

target_link_libraries(optimsocgui ${QT_LIBRARIES} ${LIBOPTIMSOCHOST_LIBRARIES} ${QWT_LIBRARIES})

install(TARGETS optimsocgui DESTINATION bin)
