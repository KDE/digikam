#
# Copyright (c) 2010-2017, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tools

                    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

set(libsendbymail_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/imageresizethread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/imageresizejob.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/mailsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/mailprocess.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/mailwizard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/mailintropage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/mailalbumspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/mailimagespage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/mailsettingspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/mailfinalpage.cpp
)

add_library(sendbymail_src OBJECT ${libsendbymail_SRCS})
