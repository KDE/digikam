#
# Copyright (c) 2010-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Network,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::XmlPatterns,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

set(unifiedplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/wsauthentication.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/manager/wstalker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/wsalbumspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/wsfinalpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/wsimagespage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/wsintropage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/wssettingspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/wsauthenticationpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/wswizard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unifiedplugin.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN_ADVANCED(NAME    Unified
                                    SOURCES ${unifiedplugin_SRCS}
)
