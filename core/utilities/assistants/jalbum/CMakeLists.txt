#
# Copyright (c) 2010-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigWidgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::Service,INTERFACE_INCLUDE_DIRECTORIES>

                    ${LIBXSLT_INCLUDE_DIR}
                    ${LIBXML2_INCLUDE_DIR}
)

add_definitions(${LIBXSLT_DEFINITIONS}
                ${LIBXML2_DEFINITIONS}
)

set(libjalbum_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/jalbumconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/jalbuminfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/jalbumgenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/jalbumintropage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/jalbumfinalpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/jalbumoutputpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/jalbumselectionpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/jalbumwizard.cpp
)

DIGIKAM_ADD_PLUGIN(JAlbumPlugin
                   ${CMAKE_CURRENT_SOURCE_DIR}/jalbumplugin.cpp
                   ${libjalbum_SRCS}
)
