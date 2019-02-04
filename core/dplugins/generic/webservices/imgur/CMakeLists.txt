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

set(imgurplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/imgurimageslist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgurtalker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgurwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgurplugin.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    ImgUr
                                    SOURCES ${imgurplugin_SRCS}
)

add_subdirectory(icons)
