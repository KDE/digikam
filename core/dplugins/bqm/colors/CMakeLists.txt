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

DIGIKAM_ADD_BQM_PLUGIN(WhiteBalance
    ${CMAKE_CURRENT_SOURCE_DIR}/whitebalanceplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/whitebalance.cpp
)

DIGIKAM_ADD_BQM_PLUGIN(IccConvert
    ${CMAKE_CURRENT_SOURCE_DIR}/iccconvertplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/iccconvert.cpp
)

DIGIKAM_ADD_BQM_PLUGIN(CurvesAdjust
    ${CMAKE_CURRENT_SOURCE_DIR}/curvesadjustplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/curvesadjust.cpp
)

DIGIKAM_ADD_BQM_PLUGIN(ChannelMixer
    ${CMAKE_CURRENT_SOURCE_DIR}/channelmixerplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/channelmixer.cpp
)

DIGIKAM_ADD_BQM_PLUGIN(BcgCorrection
    ${CMAKE_CURRENT_SOURCE_DIR}/bcgcorrectionplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bcgcorrection.cpp
)

DIGIKAM_ADD_BQM_PLUGIN(Invert
    ${CMAKE_CURRENT_SOURCE_DIR}/invertplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/invert.cpp
)

DIGIKAM_ADD_BQM_PLUGIN(HslCorrection
    ${CMAKE_CURRENT_SOURCE_DIR}/hslcorrectionplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hslcorrection.cpp
)

DIGIKAM_ADD_BQM_PLUGIN(AutoCorrection
    ${CMAKE_CURRENT_SOURCE_DIR}/autocorrectionplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/autocorrection.cpp
)

DIGIKAM_ADD_BQM_PLUGIN(BWconvert
    ${CMAKE_CURRENT_SOURCE_DIR}/bwconvertplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/bwconvert.cpp
)

DIGIKAM_ADD_BQM_PLUGIN(ColorBalance
    ${CMAKE_CURRENT_SOURCE_DIR}/colorbalanceplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/colorbalance.cpp
)
 
DIGIKAM_ADD_BQM_PLUGIN(Convert8To16
    ${CMAKE_CURRENT_SOURCE_DIR}/convert8to16plugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/convert8to16.cpp
)

DIGIKAM_ADD_BQM_PLUGIN(Convert16To8
    ${CMAKE_CURRENT_SOURCE_DIR}/convert16to8plugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/convert16to8.cpp
)
