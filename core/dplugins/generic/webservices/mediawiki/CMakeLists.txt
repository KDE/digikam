#
# Copyright (c) 2010-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2018      by Maik Qualmann, <metzpinguin at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(MacroDPlugins)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Network,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::XmlPatterns,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

set(mediawikiplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawikiplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawikitalker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawikiwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawikiwindow.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    MediaWiki
                                    SOURCES ${mediawikiplugin_SRCS}
)

add_subdirectory(icons)
