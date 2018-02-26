#
# Copyright (c) 2010-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
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

########################################################################

set(dmediaservertest_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dmediaserver_test.cpp
)

add_executable(dmediaservertest ${dmediaservertest_SRCS})

target_link_libraries(dmediaservertest
                      digikamcore

                      Qt5::Widgets
                      Qt5::Gui
                      Qt5::Core
                      Qt5::Test
)

########################################################################

set(dmediaserver_dlg_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dmediaserver_dlg.cpp
)

add_executable(dmediaserver_dlg ${dmediaserver_dlg_SRCS})

target_link_libraries(dmediaserver_dlg
                      digikamcore

                      Qt5::Widgets
                      Qt5::Gui
                      Qt5::Core
                      Qt5::Test
)

########################################################################
# CLI test tool from Platinum SDK

#set(filemediaserver_SRCS
#    ${CMAKE_CURRENT_SOURCE_DIR}/../../utilities/mediaserver/upnpsdk/Platinum/Source/Tests/FileMediaServer/FileMediaServerTest.cpp
#)

#add_executable(filemediaserver ${filemediaserver_SRCS} ${upnpsdk_SRCS})

#target_link_libraries(filemediaserver
#                      digikamcore
#)
