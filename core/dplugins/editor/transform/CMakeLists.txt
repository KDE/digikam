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

#DIGIKAM_ADD_EDITOR_PLUGIN(FreeRotationTool
#    ${CMAKE_CURRENT_SOURCE_DIR}/freerotationtoolplugin.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/freerotationtool.cpp
#)

#DIGIKAM_ADD_EDITOR_PLUGIN(ShearTool
#    ${CMAKE_CURRENT_SOURCE_DIR}/sheartoolplugin.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/sheartool.cpp
#)

#DIGIKAM_ADD_EDITOR_PLUGIN(ResizeTool
#    ${CMAKE_CURRENT_SOURCE_DIR}/resizetoolplugin.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/resizetool.cpp
#)

DIGIKAM_ADD_EDITOR_PLUGIN(PerspectiveTool
    ${CMAKE_CURRENT_SOURCE_DIR}/perspective/perspectivetoolplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/perspective/perspectivetool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/perspective/perspectivewidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/perspective/triangle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/perspective/matrix.cpp
)

#DIGIKAM_ADD_EDITOR_PLUGIN(RatioCropTool
#    ${CMAKE_CURRENT_SOURCE_DIR}/imageselectionwidget.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/ratiocroptoolplugin.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/ratiocroptool.cpp
#)

#if(Lqr-1_FOUND)
#
#    DIGIKAM_ADD_EDITOR_PLUGIN(ContentAwareResizeTool
#        ${CMAKE_CURRENT_SOURCE_DIR}/contentawareresizetoolplugin.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/contentawareresizetool.cpp
#       )
#
#endif()
