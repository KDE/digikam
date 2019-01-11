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

# DIGIKAM_ADD_BQM_PLUGIN(BQMAntiVignettingPlugin
#     ${CMAKE_CURRENT_SOURCE_DIR}/antivignettingplugin.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/antivignetting.cpp
# )

# DIGIKAM_ADD_BQM_PLUGIN(BQMBlurPlugin
#     ${CMAKE_CURRENT_SOURCE_DIR}/blurplugin.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/blur.cpp
# )
#
# DIGIKAM_ADD_BQM_PLUGIN(BQMLocalContrastPlugin
#     ${CMAKE_CURRENT_SOURCE_DIR}/localcontrastplugin.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/localcontrast.cpp
# )
#
# DIGIKAM_ADD_BQM_PLUGIN(BQMNoiseReductionPlugin
#     ${CMAKE_CURRENT_SOURCE_DIR}/noisereductionplugin.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/noisereduction.cpp
# )
#
# DIGIKAM_ADD_BQM_PLUGIN(BQMRedEyeCorrectionPlugin
#     ${CMAKE_CURRENT_SOURCE_DIR}/redeyecorrectionplugin.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/redeyecorrection.cpp
# )
#
# DIGIKAM_ADD_BQM_PLUGIN(BQMRestorationPlugin
#     ${CMAKE_CURRENT_SOURCE_DIR}/restorationplugin.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/restoration.cpp
# )
#
# DIGIKAM_ADD_BQM_PLUGIN(BQMSharpenPlugin
#     ${CMAKE_CURRENT_SOURCE_DIR}/sharpenplugin.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/sharpen.cpp
# )
#
# if(LensFun_FOUND)
#     include_directories(${LENSFUN_INCLUDE_DIRS})
#    DIGIKAM_ADD_BQM_PLUGIN(BQMConvertToJPEGPlugin
#         ${CMAKE_CURRENT_SOURCE_DIR}/lensautofixplugin.cpp
#         ${CMAKE_CURRENT_SOURCE_DIR}/lensautofix.cpp
#    )
#
# endif()
