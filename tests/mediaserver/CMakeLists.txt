#
# Copyright (c) 2010-2017, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories(
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Test,INTERFACE_INCLUDE_DIRECTORIES>
)

set(mediaservertest_SRCS
    main.cpp
    mediaserver_introductionwindow.cpp
)

qt5_wrap_ui(mediaservertest_SRCS
            ${CMAKE_CURRENT_SOURCE_DIR}/mediaserver_introductionwindow.ui
)

add_executable(mediaservertest ${mediaservertest_SRCS})

target_link_libraries(mediaservertest
                      digikamcore
                      libdng

                      Qt5::Widgets
                      Qt5::Gui
                      Qt5::Core
                      Qt5::Test
)
