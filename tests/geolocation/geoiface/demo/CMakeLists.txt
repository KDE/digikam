#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(geoiface_demo_SRCS
    demo-main.cpp
    myimageitem.cpp
    mytreewidget.cpp
    mydragdrophandler.cpp
    mainwindow.cpp
)

add_executable(geoiface_demo ${geoiface_demo_SRCS})

target_link_libraries(geoiface_demo
                      digikamcore

                      Qt5::Widgets
                      Qt5::Gui
                      Qt5::Concurrent

                      KF5::KExiv2

                      KF5::ConfigCore
                      KF5::CoreAddons
                      KF5::I18n)
