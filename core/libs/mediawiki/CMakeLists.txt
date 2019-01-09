#
# Copyright (c) 2010-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2018      by Maik Qualmann, <metzpinguin at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Network,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::XmlPatterns,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

set(libmediawiki_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki_image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki_imageinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki_iface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki_login.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki_logout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki_queryimageinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki_queryimages.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki_querysiteinfogeneral.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki_generalinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki_querysiteinfousergroups.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki_queryrevision.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki_queryinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki_edit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki_job.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki_usergroup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki_revision.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki_page.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki_parse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki_protection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki_upload.cpp
)

add_library(mediawiki_src OBJECT ${libmediawiki_SRCS})
