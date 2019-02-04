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
                    $<TARGET_PROPERTY:KF5::KIOCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::KIOFileWidgets,INTERFACE_INCLUDE_DIRECTORIES>
)

set(filetransferplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/ftexportwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ftexportwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ftimportwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ftimportwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ftplugin.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    FileTransfer
                                    SOURCES ${filetransferplugin_SRCS}
)
