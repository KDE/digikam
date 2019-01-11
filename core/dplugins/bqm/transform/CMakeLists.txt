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

DIGIKAM_ADD_BQM_PLUGIN(BQMCropPlugin
    ${CMAKE_CURRENT_SOURCE_DIR}/cropplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/crop.cpp
)

DIGIKAM_ADD_BQM_PLUGIN(BQMFlipPlugin
    ${CMAKE_CURRENT_SOURCE_DIR}/flipplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/flip.cpp
)

DIGIKAM_ADD_BQM_PLUGIN(BQMResizePlugin
    ${CMAKE_CURRENT_SOURCE_DIR}/resizeplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/resize.cpp
)

DIGIKAM_ADD_BQM_PLUGIN(BQMRotatePlugin
    ${CMAKE_CURRENT_SOURCE_DIR}/rotateplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rotate.cpp
)
