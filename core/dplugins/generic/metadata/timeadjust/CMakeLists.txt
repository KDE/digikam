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

set(timeadjustplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/timeadjustdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/timeadjustlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/timeadjusttask.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/timeadjustthread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/timeadjustplugin.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN_ADVANCED(NAME    TimeAdjust
                                    SOURCES ${timeadjustplugin_SRCS}
)
