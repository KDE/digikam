#
# Copyright (c) 2010-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(MacroDPlugins)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/
                    ${CMAKE_CURRENT_SOURCE_DIR}/tools

                    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

set(jalbumplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/jalbumsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/jalbumgenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/jalbumintropage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/jalbumfinalpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/jalbumoutputpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/jalbumselectionpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/jalbumwizard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tools/jalbumjar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/jalbumplugin.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN_ADVANCED(NAME    JAlbum
                                    SOURCES ${jalbumplugin_SRCS}
)
