#
# Copyright (c) 2010-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(dmediaservertest_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dmediaserver_test.cpp
)

add_executable(dmediaservertest ${dmediaservertest_SRCS})

target_link_libraries(dmediaservertest
                      digikamcore
                      mediaserverbackend

                      Qt5::Test
)

########################################################################
# CLI test tool from Platinum SDK

set(filemediaserver_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/../upnpsdk/Platinum/Source/Tests/FileMediaServer/FileMediaServerTest.cpp
)

add_executable(filemediaserver ${filemediaserver_SRCS})

target_link_libraries(filemediaserver
                      digikamcore
                      mediaserverbackend
)
