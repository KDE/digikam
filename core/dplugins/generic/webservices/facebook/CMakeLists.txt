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

set(facebookplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/fbmpform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fbnewalbumdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fbplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fbtalker.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/fbtalker_wizard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fbwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fbwindow.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN_ADVANCED(NAME    FaceBook
                                    SOURCES ${facebookplugin_SRCS}
)

add_subdirectory(icons)
