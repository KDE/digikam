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

DIGIKAM_ADD_EDITOR_PLUGIN(ColorFxTool
    ${CMAKE_CURRENT_SOURCE_DIR}/colorfxtoolplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/colorfxtool.cpp
)

#DIGIKAM_ADD_EDITOR_PLUGIN(CharcoalTool
#    ${CMAKE_CURRENT_SOURCE_DIR}/charcoaltoolplugin.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/charcoaltool.cpp
#)

#DIGIKAM_ADD_EDITOR_PLUGIN(EmbossTool
#    ${CMAKE_CURRENT_SOURCE_DIR}/embosstoolplugin.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/embosstool.cpp
#)

#DIGIKAM_ADD_EDITOR_PLUGIN(OilPaintTool
#    ${CMAKE_CURRENT_SOURCE_DIR}/oilpainttoolplugin.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/oilpainttool.cpp
#)

DIGIKAM_ADD_EDITOR_PLUGIN(BlurFxTool
    ${CMAKE_CURRENT_SOURCE_DIR}/blurfxtoolplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/blurfxtool.cpp
)

DIGIKAM_ADD_EDITOR_PLUGIN(DistortionFxTool
    ${CMAKE_CURRENT_SOURCE_DIR}/distortionfxtoolplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/distortionfxtool.cpp
)

#DIGIKAM_ADD_EDITOR_PLUGIN(RainDropTool
#    ${CMAKE_CURRENT_SOURCE_DIR}/raindroptoolplugin.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/raindroptool.cpp
#)

#DIGIKAM_ADD_EDITOR_PLUGIN(FilmGrainTool
#    ${CMAKE_CURRENT_SOURCE_DIR}/filmgraintoolplugin.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/filmgraintool.cpp
#)
