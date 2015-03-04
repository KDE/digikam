#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libprogressmanager_SRCS
    overlaywidget.cpp
    progressview.cpp
    progressmanager.cpp
    statusbarprogresswidget.cpp
    statusprogressbar.cpp
)

add_library(progressmanager STATIC ${libprogressmanager_SRCS})

target_link_libraries(progressmanager

                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets
                      
                      KF5::KDcraw
)