#
# Copyright (c) 2015-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories($<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::Sane,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
)

set(printplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/print/printhelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/print/printoptionspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/print/printconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/print/printplugin.cpp
)

ki18n_wrap_ui(printplugin_SRCS
              ${CMAKE_CURRENT_SOURCE_DIR}/print/printoptionspage.ui
)

DIGIKAM_ADD_EDITOR_PLUGIN(PrintTool ${printplugin_SRCS})
