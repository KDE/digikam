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

DIGIKAM_ADD_BQM_PLUGIN(ConvertToJPEG
    ${CMAKE_CURRENT_SOURCE_DIR}/converttojpegplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/convert2jpeg.cpp
)

DIGIKAM_ADD_BQM_PLUGIN(ConvertToDNG
    ${CMAKE_CURRENT_SOURCE_DIR}/converttodngplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/convert2dng.cpp
)

DIGIKAM_ADD_BQM_PLUGIN(ConvertToTIFF
    ${CMAKE_CURRENT_SOURCE_DIR}/converttotiffplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/convert2tiff.cpp
)

DIGIKAM_ADD_BQM_PLUGIN(ConvertToPNG
    ${CMAKE_CURRENT_SOURCE_DIR}/converttopngplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/convert2png.cpp
)

DIGIKAM_ADD_BQM_PLUGIN(ConvertToPGF
    ${CMAKE_CURRENT_SOURCE_DIR}/converttopgfplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/convert2pgf.cpp
)

if(Jasper_FOUND)

    DIGIKAM_ADD_BQM_PLUGIN(ConvertToJP2lugin
        ${CMAKE_CURRENT_SOURCE_DIR}/converttojp2plugin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/convert2jp2.cpp
    )

endif()
