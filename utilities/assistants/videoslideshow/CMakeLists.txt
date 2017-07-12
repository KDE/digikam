#
# Copyright (c) 2010-2017, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::Service,INTERFACE_INCLUDE_DIRECTORIES>
)

set(libvideoslideshow_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/vidslidewizard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/vidslideintropage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/vidslidealbumspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/vidslideimagespage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/vidslidevideopage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/vidslideoutputpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/vidslidefinalpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/vidslidesettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/vidslidethread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/vidslidetask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/player/vidplayerdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/player/mediaplayerview.cpp
)

add_library(videoslideshow_src OBJECT ${libvideoslideshow_SRCS})
