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

set(filecopyplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/fcexportwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fcexportwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fcplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fcthread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fctask.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    FileCopy
                                    SOURCES ${filecopyplugin_SRCS}
)
