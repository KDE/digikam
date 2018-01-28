#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Network,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

# OAuth2 library ---------------------------------------------------

# The o2 library does not adhere to the flags we use
remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY)
# It also does not export symbols, so export all by default
unset(CMAKE_CXX_VISIBILITY_PRESET)

if(WIN32)
    add_definitions(-DO2_DLL_EXPORT)
endif()

# Copied from o2/src/CMakeLists.txt
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(libwso2_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o2reply.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o2replyserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o2requestor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o2simplecrypt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o0settingsstore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o0baseauth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o0abstractstore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o0globals.h
    # Enable when needed
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o1requestor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o1timedreply.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o1smugmug.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o1twitter.h
    #${CMAKE_CURRENT_SOURCE_DIR}/o2/src/oxtwitter.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o1dropbox.h
    #${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o1flickr.h
    #${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o2gft.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o2facebook.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o2skydrive.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/o2/src/o2hubic.cpp
)

# Base classes ------------------------------------------------------------------

set(libwebservices_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/base/comboboxdelegate.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/base/comboboxintermediate.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/base/exportutils.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/base/settingswidget.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/base/newalbumdialog.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/base/selectuserdlg.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/base/tooldialog.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/base/logindialog.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/manager/wssettings.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/wizard/wsalbumspage.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/wizard/wsfinalpage.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/wizard/wsimagespage.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/wizard/wsintropage.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/wizard/wssettingspage.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/wizard/wswizard.cpp
)

# Flickr export ----------------------------------------------------------------

set(libwsflickr_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/flickr/flickrlist.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/flickr/flickrmpform.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/flickr/flickrtalker.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/flickr/flickrwidget.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/flickr/flickrnewalbum.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/flickr/flickrtalker.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/flickr/flickrwindow.cpp
)

# Imgur export ----------------------------------------------------------------

set(libwsimgur_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/imgur/imgurapi3.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/imgur/imgurimageslist.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/imgur/imgurwindow.cpp
)

# DropBox export --------------------------------------------------------------

set(libwsdropbox_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/dropbox/dbalbum.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/dropbox/dbmpform.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/dropbox/dbtalker.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/dropbox/dbwidget.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/dropbox/dbwindow.cpp
)

# SmugMug export -------------------------------------------------------------

set(libwssmugmug_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/smugmug/smugalbum.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/smugmug/smugmpform.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/smugmug/smugtalker.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/smugmug/smugwidget.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/smugmug/smugwindow.cpp
)

# WikiMedia export ------------------------------------------------------------

if(KF5MediaWiki_FOUND)

    set(libwswikimedia_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/wikimedia/wmwindow.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/wikimedia/wmwidget.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/wikimedia/wmtalker.cpp
    )

endif()

# ------------------------------------------------------------------------------

add_library(webservices_src OBJECT ${libwebservices_SRCS}
                                   ${libwso2_SRCS}
                                   ${libwsflickr_SRCS}
                                   ${libwsimgur_SRCS}
                                   ${libwsdropbox_SRCS}
                                   ${libwssmugmug_SRCS}
                                   ${libwswikimedia_SRCS}
)
