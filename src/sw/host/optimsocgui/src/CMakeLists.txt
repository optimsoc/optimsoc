# add all *.cpp files here
SET(optimsocgui_SRCS
    main.cpp
    mainwindow.cpp
    aboutdialog.cpp
    configuredialog.cpp
    executionchart.cpp
    executionchartplots.cpp
    executionchartelements.cpp
    executionchartcreators.cpp
    systemoverviewwidget.cpp
    componentitem.cpp
    nocrouteritem.cpp
    noclinkitem.cpp
    tileitem.cpp
    memorytileitem.cpp
    computetileitem.cpp
    externaltileitem.cpp
    writememorydialog.cpp
    hardwareinterface.cpp
    memorytile.cpp
    computetile.cpp
    externaltile.cpp
    tile.cpp
    optimsocsystem.cpp
    optimsocsystemfactory.cpp
    nocrouter.cpp
    meshnoc.cpp
    noc.cpp
    meshnoclayout.cpp
    noclayout.cpp
    optimsocsystemscene.cpp
    traceevents.cpp
    plotspectrogram.cpp
    qcustomplot.cpp
)

# add all *.ui files here
SET(optimsocgui_UIS
    mainwindow.ui
    aboutdialog.ui
    configuredialog.ui
    executionchart.ui
    writememorydialog.ui
    plotspectrogram.ui
)

# add all files which contain classes using the QT_OBJECT macro here
SET(optimsocgui_MOC_HEADERS
    mainwindow.h
    aboutdialog.h
    configuredialog.h
    executionchart.h
    executionchartplots.h
    executionchartelements.h
    executionchartcreators.h
    systemoverviewwidget.h
    memorytileitem.h
    computetileitem.h
    externaltileitem.h
    tileitem.h
    componentitem.h
    writememorydialog.h
    hardwareinterface.h
    memorytile.h
    computetile.h
    externaltile.h
    tile.h
    optimsocsystem.h
    optimsocsystemfactory.h
    nocrouter.h
    meshnoc.h
    noc.h
    meshnoclayout.h
    noclayout.h
    optimsocsystemscene.h
    traceevents.h
    plotspectrogram.h
    qcustomplot.h
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

# generate ui_*.h files and include them
QT4_WRAP_UI(optimsocgui_UIS_H ${optimsocgui_UIS})
include_directories(
  ${LIBOPTIMSOCHOST_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
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
