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

set(hupnpav_servertest_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/hupnpserver_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hupnpserver_window.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hupnpserver_introductionwindow.cpp
)

qt5_wrap_ui(hupnpav_servertest_SRCS
            ${CMAKE_CURRENT_SOURCE_DIR}/hupnpserver_window.ui
            ${CMAKE_CURRENT_SOURCE_DIR}/hupnpserver_introductionwindow.ui
)

add_executable(hupnpav_servertest ${hupnpav_servertest_SRCS})

target_link_libraries(hupnpav_servertest
                      digikamcore
                      libdng

                      Qt5::Widgets
                      Qt5::Gui
                      Qt5::Core
                      Qt5::Test
)

########################################################################

set(dmediaservertest_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dmediaserver_test.cpp
)

add_executable(dmediaservertest ${dmediaservertest_SRCS})

target_link_libraries(dmediaservertest
                      digikamcore
                      libdng

                      Qt5::Widgets
                      Qt5::Gui
                      Qt5::Core
                      Qt5::Test
)
