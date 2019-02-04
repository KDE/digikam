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
                    $<TARGET_PROPERTY:KF5::Vkontakte,INTERFACE_INCLUDE_DIRECTORIES>
)

set(vkontakteplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/vkalbumchooser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vkauthwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vknewalbumdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vkplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vkwindow.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    VKontakte
                           SOURCES ${vkontakteplugin_SRCS}
                           DEPENDS KF5::Vkontakte
)
