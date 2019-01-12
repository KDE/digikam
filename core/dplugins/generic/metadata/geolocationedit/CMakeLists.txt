#
# Copyright (c) 2010-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Network,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

DIGIKAM_ADD_GENERIC_PLUGIN(GeolocationEdit
    ${CMAKE_CURRENT_SOURCE_DIR}/searches/searchbackend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/searches/searchresultmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/searches/searchresultmodelhelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/searches/searchwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialog/gpsitemdetails.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialog/geolocationedit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialog/gpsgeoifacemodelhelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kmlexport/geodataparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kmlexport/kmlgpsdataparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kmlexport/kmlwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kmlexport/kmlexport.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/geolocationeditplugin.cpp)
