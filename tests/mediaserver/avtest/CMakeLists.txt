#
# Copyright (c) 2010-2017, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(AvTest_SRCS
    controlpoint_navigator.cpp
    dataitem_display.cpp
    mediamanager_dialog.cpp
    mediaserver_window.cpp
    controlpoint_navigatoritem.cpp
    main.cpp
    mediarenderer_connectionwindow.cpp
    rendererconnections.cpp
    controlpoint_window.cpp
    mainwindow.cpp
    mediarenderer_window.cpp
)

qt5_wrap_ui(AvTest_FORMS
            controlpoint.ui
            mainwindow.ui
            mediarenderer_connectionwindow.ui
            mediaserver_window.ui
            invokeactiondialog.ui
            mediamanager_dialog.ui
            mediarenderer_window.ui
)

set(SRCS ${AvTest_SRCS} ${AvTest_FORMS})

add_executable(avtest ${SRCS})

target_link_libraries(avtest
                      HUpnp
                      HUpnpAv
                      Qt5::Core
                      Qt5::Widgets
                      Qt5::Network
                      Qt5::Xml
                      Qt5::Test
)
