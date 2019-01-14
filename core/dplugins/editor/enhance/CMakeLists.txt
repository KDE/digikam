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

DIGIKAM_ADD_EDITOR_PLUGIN(RestorationTool
    ${CMAKE_CURRENT_SOURCE_DIR}/restorationtoolplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/restorationtool.cpp
)

DIGIKAM_ADD_EDITOR_PLUGIN(LocalContrastTool
    ${CMAKE_CURRENT_SOURCE_DIR}/localcontrasttoolplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/localcontrasttool.cpp
)

DIGIKAM_ADD_EDITOR_PLUGIN(NoiseReductionTool
    ${CMAKE_CURRENT_SOURCE_DIR}/noisereductiontoolplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/noisereductiontool.cpp
)

DIGIKAM_ADD_EDITOR_PLUGIN(SharpenTool
    ${CMAKE_CURRENT_SOURCE_DIR}/sharpentoolplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sharpentool.cpp
)

#DIGIKAM_ADD_EDITOR_PLUGIN(BlurTool
#    ${CMAKE_CURRENT_SOURCE_DIR}/blurtoolplugin.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/blurtool.cpp
#)

#DIGIKAM_ADD_EDITOR_PLUGIN(RedEyeTool
#    ${CMAKE_CURRENT_SOURCE_DIR}/redeyetoolplugin.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/redeyetool.cpp
#)

#DIGIKAM_ADD_EDITOR_PLUGIN(AntivignettingTool
#    ${CMAKE_CURRENT_SOURCE_DIR}/antivignettingtoolplugin.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/antivignettingtool.cpp
#)

#DIGIKAM_ADD_EDITOR_PLUGIN(LensDistortionTool
#    ${CMAKE_CURRENT_SOURCE_DIR}/lensdistorsiontoolplugin.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/lensdistortiontool.cpp
#)

#DIGIKAM_ADD_EDITOR_PLUGIN(HotpixelsTool
#    ${CMAKE_CURRENT_SOURCE_DIR}/hotpixels/weights.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/hotpixels/blackframeparser.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/hotpixels/blackframelistview.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/hotpixels/hotpixelfixer.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/hotpixels/hotpixelstoolplugin.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/hotpixels/hotpixelstool.cpp
#)

#if(LensFun_FOUND)
#
#    include_directories(${LENSFUN_INCLUDE_DIRS})
#
#    DIGIKAM_ADD_EDITOR_PLUGIN(HotpixelsTool
#        ${CMAKE_CURRENT_SOURCE_DIR}/lensautofixtoolplugin.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/lensautofixtool.cpp
#    )
#
#endif()

# Not yet finalized
#DIGIKAM_ADD_EDITOR_PLUGIN(HealingCloneTool
#    ${CMAKE_CURRENT_SOURCE_DIR}/healingclonetoolplugin.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/healingclonetool.cpp
#)
