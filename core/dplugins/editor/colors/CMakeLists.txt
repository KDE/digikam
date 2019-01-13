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

DIGIKAM_ADD_EDITOR_PLUGIN(BcgTool
    ${CMAKE_CURRENT_SOURCE_DIR}/bcgtoolplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bcgtool.cpp
)

DIGIKAM_ADD_EDITOR_PLUGIN(AdjustCurvesTool
    ${CMAKE_CURRENT_SOURCE_DIR}/adjustcurvestoolplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/adjustcurvestool.cpp
)

DIGIKAM_ADD_EDITOR_PLUGIN(AdjustLevelsTool
    ${CMAKE_CURRENT_SOURCE_DIR}/adjustlevelstoolplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/adjustlevelstool.cpp
)

DIGIKAM_ADD_EDITOR_PLUGIN(AutoCorrectionTool
    ${CMAKE_CURRENT_SOURCE_DIR}/autocorrectiontoolplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/autocorrectiontool.cpp
)

DIGIKAM_ADD_EDITOR_PLUGIN(BWSepiaTool
    ${CMAKE_CURRENT_SOURCE_DIR}/bwsepiatoolplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bwsepiatool.cpp
)

