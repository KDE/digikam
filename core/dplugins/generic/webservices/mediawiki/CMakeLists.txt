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
                    $<TARGET_PROPERTY:KF5::CoreAddons,INTERFACE_INCLUDE_DIRECTORIES>
)

# --------------------------------------------------------------------------------

set(libmediawiki_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_imageinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_iface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_login.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_logout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_queryimageinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_queryimages.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_querysiteinfogeneral.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_generalinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_querysiteinfousergroups.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_queryrevision.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_queryinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_edit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_job.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_usergroup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_revision.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_page.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_parse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_protection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/mediawiki_upload.cpp
)

add_library(mediawikibackend STATIC ${libmediawiki_SRCS})

# --------------------------------------------------------------------------------

set(mediawikiplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawikiplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawikitalker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawikiwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawikiwindow.cpp
)

DIGIKAM_ADD_GENERIC_PLUGIN(NAME    MediaWiki
                           SOURCES ${mediawikiplugin_SRCS}
                           DEPENDS mediawikibackend
)

add_subdirectory(icons)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
