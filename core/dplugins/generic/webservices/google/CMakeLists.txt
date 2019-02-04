#
# Copyright (c) 2010-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2018      by Maik Qualmann, <metzpinguin at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

set(googleplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/gsnewalbumdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gsplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gsreplacedlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gstalkerbase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gswidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gswindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gdrive/gdmpform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gdrive/gdtalker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gphoto/gpmpform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gphoto/gptalker.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN_ADVANCED(NAME    Google
                                    SOURCES ${googleplugin_SRCS}
)

add_subdirectory(icons)
