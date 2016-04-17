#
# Copyright (c) 2010-2015, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(libgeoiface_SRCS backends/mapbackend.cpp
                     backends/htmlwidget.cpp
                     backends/backendmarble.cpp
                     backends/backendmarblelayer.cpp
                     backends/backendgooglemaps.cpp

                     abstractmarkertiler.cpp
                     dragdrophandler.cpp
                     geocoordinates.cpp
                     groupstatecomputer.cpp
                     itemmarkertiler.cpp
                     geoiface_common.cpp
                     lookupaltitude.cpp
                     lookupaltitudegeonames.cpp
                     lookupfactory.cpp
                     mapwidget.cpp
                     modelhelper.cpp
                     placeholderwidget.cpp
                     tilegrouper.cpp
                     tileindex.cpp
                     trackreader.cpp
                     tracks.cpp
)

include_directories(
    ${MARBLE_INCLUDE_DIR}

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::WebKitWidgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Network,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(geoiface_src OBJECT ${libgeoiface_SRCS})
