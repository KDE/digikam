#
# Copyright (c) 2010-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2018      by Maik Qualmann, <metzpinguin at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

set(onedriveplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/odmpform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/odnewalbumdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/odplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/odtalker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/odwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/odwindow.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN_ADVANCED(NAME    OneDrive
                                    SOURCES ${onedriveplugin_SRCS}
)

add_subdirectory(icons)
