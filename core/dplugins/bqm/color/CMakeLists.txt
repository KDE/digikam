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

DIGIKAM_ADD_BQM_PLUGIN(BQMWhiteBalancePlugin
    ${CMAKE_CURRENT_SOURCE_DIR}/whitebalanceplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/whitebalance.cpp
)

# DIGIKAM_ADD_BQM_PLUGIN(BQMIccConvertPlugin
#     ${CMAKE_CURRENT_SOURCE_DIR}/iccconvertplugin.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/iccconvert.cpp
# )
# 
# DIGIKAM_ADD_BQM_PLUGIN(BQMCurvesAdjustPlugin
#     ${CMAKE_CURRENT_SOURCE_DIR}/curvesadjustplugin.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/curvesadjust.cpp
# )
# 
# DIGIKAM_ADD_BQM_PLUGIN(BQMChannelMixerPlugin
#     ${CMAKE_CURRENT_SOURCE_DIR}/channelmixerplugin.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/channelmixer.cpp
# )
# 
# DIGIKAM_ADD_BQM_PLUGIN(BQMBcgCorrectionPlugin
#     ${CMAKE_CURRENT_SOURCE_DIR}/bcgcorrectionplugin.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/bcgcorrection.cpp
# )
# 
# DIGIKAM_ADD_BQM_PLUGIN(BQMInvertPlugin
#     ${CMAKE_CURRENT_SOURCE_DIR}/invertplugin.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/invert.cpp
# )
# 
# DIGIKAM_ADD_BQM_PLUGIN(BQMHslCorrectionPlugin
#     ${CMAKE_CURRENT_SOURCE_DIR}/hslcorrectionplugin.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/hslcorrection.cpp
# )
# 
# DIGIKAM_ADD_BQM_PLUGIN(BQMAutoCorrectionPlugin
#     ${CMAKE_CURRENT_SOURCE_DIR}/autocorrectionplugin.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/autocorrection.cpp
# )
# 
# DIGIKAM_ADD_BQM_PLUGIN(BQMBWconvertPlugin
#     ${CMAKE_CURRENT_SOURCE_DIR}/bwconvertplugin.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/bwconvert.cpp
# )
# 
# DIGIKAM_ADD_BQM_PLUGIN(BQMColorBalancePlugin
#     ${CMAKE_CURRENT_SOURCE_DIR}/colorbalanceplugin.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/colorbalance.cpp
# )
# 
# DIGIKAM_ADD_BQM_PLUGIN(BQMConvert8To16Plugin
#     ${CMAKE_CURRENT_SOURCE_DIR}/convert8to16plugin.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/convert8to16.cpp
# )
# 
# DIGIKAM_ADD_BQM_PLUGIN(BQMConvert16To8Plugin
#     ${CMAKE_CURRENT_SOURCE_DIR}/convert16to8plugin.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/convert16to8.cpp
# )
