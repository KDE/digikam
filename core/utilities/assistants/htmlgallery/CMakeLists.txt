#
# Copyright (c) 2010-2017, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Concurrent,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigWidgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::Service,INTERFACE_INCLUDE_DIRECTORIES>

                    ${LIBXSLT_INCLUDE_DIR}
                    ${LIBXML2_INCLUDE_DIR}
)

add_definitions(${LIBXSLT_DEFINITIONS}
                ${LIBXML2_DEFINITIONS}
)

set(libhtmlgallery_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/abstractthemeparameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/intthemeparameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/listthemeparameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/stringthemeparameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parameters/colorthemeparameter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/invisiblebuttongroup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlintropage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlselectionpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlthemepage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmloutputpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlfinalpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlimagesettingspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlparameterspage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/wizard/htmlwizard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/galleryxmlutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/gallerynamehelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/galleryelementfunctor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/galleryconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/galleryelement.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/gallerytheme.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/galleryinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generator/gallerygenerator.cpp
)

add_library(htmlgallery_src OBJECT ${libhtmlgallery_SRCS})
