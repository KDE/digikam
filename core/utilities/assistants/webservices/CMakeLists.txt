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
                    $<TARGET_PROPERTY:Qt5::XmlPatterns,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::XmlGui,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

if(KF5KIO_FOUND)

    include_directories($<TARGET_PROPERTY:KF5::KIOCore,INTERFACE_INCLUDE_DIRECTORIES>
                        $<TARGET_PROPERTY:KF5::KIOFileWidgets,INTERFACE_INCLUDE_DIRECTORIES>
    )

endif()
                      
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
    ${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o2reply.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o2replyserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o2requestor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o2simplecrypt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o0settingsstore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o0baseauth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o0abstractstore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o0globals.h
    # Enable when needed
    ${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o1requestor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o1timedreply.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o1smugmug.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o1twitter.h
    #${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/oxtwitter.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o1dropbox.h
    #${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o1flickr.h
    #${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o2gft.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o2facebook.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o2skydrive.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o2hubic.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/common/o2/src/o2uber.cpp
)

# Helper classes ------------------------------------------------------------------

set(libwebservices_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/common/wscomboboxdelegate.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/common/wscomboboxintermediate.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/common/wstoolutils.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/common/wssettingswidget.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/common/wsnewalbumdialog.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/common/wsselectuserdlg.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/common/wstooldialog.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/common/wslogindialog.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/common/manager/wssettings.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/common/wizard/wsalbumspage.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/common/wizard/wsfinalpage.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/common/wizard/wsimagespage.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/common/wizard/wsintropage.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/common/wizard/wssettingspage.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/common/wizard/wswizard.cpp
)

# Flickr tool ----------------------------------------------------------------

set(libwsflickr_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/flickr/flickrlist.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/flickr/flickrmpform.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/flickr/flickrtalker.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/flickr/flickrwidget.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/flickr/flickrnewalbumdlg.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/flickr/flickrtalker.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/flickr/flickrwindow.cpp
)

# Imgur tool ----------------------------------------------------------------

set(libwsimgur_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/imgur/imgurtalker.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/imgur/imgurimageslist.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/imgur/imgurwindow.cpp
)

# DropBox tool --------------------------------------------------------------

set(libwsdropbox_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/dropbox/dbnewalbumdlg.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/dropbox/dbmpform.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/dropbox/dbtalker.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/dropbox/dbwidget.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/dropbox/dbwindow.cpp
)

# SmugMug tool -------------------------------------------------------------

set(libwssmugmug_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/smugmug/smugnewalbumdlg.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/smugmug/smugmpform.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/smugmug/smugtalker.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/smugmug/smugwidget.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/smugmug/smugwindow.cpp
)

# ImageShack tool -------------------------------------------------------------

set(libwsimageshack_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/imageshack/imageshacksession.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/imageshack/imageshacknewalbumdlg.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/imageshack/imageshackmpform.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/imageshack/imageshacktalker.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/imageshack/imageshackwidget.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/imageshack/imageshackwindow.cpp
)

# FaceBook tool -------------------------------------------------------------

set(libwsfacebook_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/facebook/fbnewalbumdlg.cpp
                       ${CMAKE_CURRENT_SOURCE_DIR}/facebook/fbmpform.cpp
                       ${CMAKE_CURRENT_SOURCE_DIR}/facebook/fbtalker.cpp
                       ${CMAKE_CURRENT_SOURCE_DIR}/facebook/fbwidget.cpp
                       ${CMAKE_CURRENT_SOURCE_DIR}/facebook/fbwindow.cpp
)

# Rajce tool ----------------------------------------------------------------

set(libwsrajce_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/rajce/rajcenewalbumdlg.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/rajce/rajcempform.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/rajce/rajcecommand.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/rajce/rajcetalker.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/rajce/rajcesession.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/rajce/rajcealbum.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/rajce/rajcewidget.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/rajce/rajcewindow.cpp
)


# Google tool ----------------------------------------------------------------

set(libwsgoogle_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/google/gdrive/gdmpform.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/google/gdrive/gdtalker.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/google/gphoto/gpmpform.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/google/gphoto/gptalker.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/google/gstalkerbase.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/google/gsreplacedlg.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/google/gsnewalbumdlg.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/google/gswindow.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/google/gswidget.cpp
)

# YandexFotki tool ----------------------------------------------------------------

set(libwsyandexfotki_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/yandexfotki/yfalbum.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/yandexfotki/yfauth.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/yandexfotki/yfrsa.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/yandexfotki/yftalker.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/yandexfotki/yfphoto.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/yandexfotki/yfnewalbumdlg.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/yandexfotki/yfwindow.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/yandexfotki/yfwidget.cpp
)

# Piwigo tool -------------------------------------------------------------

set(libwspiwigo_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/piwigo/piwigologindlg.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/piwigo/piwigotalker.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/piwigo/piwigosession.cpp
                     ${CMAKE_CURRENT_SOURCE_DIR}/piwigo/piwigowindow.cpp
)

# MediaWiki tool ------------------------------------------------------------

if(KF5MediaWiki_FOUND)

    set(libwsmediawiki_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/mediawikiwindow.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/mediawikiwidget.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/mediawikitalker.cpp
    )

endif()

# VKontakte tool ------------------------------------------------------------

if(KF5Vkontakte_FOUND)

    set(libwsvkontakte_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/vkontakte/vkwindow.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/vkontakte/vknewalbumdlg.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/vkontakte/vkalbumchooser.cpp
                            ${CMAKE_CURRENT_SOURCE_DIR}/vkontakte/vkauthwidget.cpp
    )

endif()


# FileTransfer tool ---------------------------------------------------------

if(KF5KIO_FOUND)

    set(libwsfiletransfer_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/filetransfer/ftexportwindow.cpp
                               ${CMAKE_CURRENT_SOURCE_DIR}/filetransfer/ftexportwidget.cpp
                               ${CMAKE_CURRENT_SOURCE_DIR}/filetransfer/ftimportwindow.cpp
                               ${CMAKE_CURRENT_SOURCE_DIR}/filetransfer/ftimportwidget.cpp
    )

endif()
    
# ------------------------------------------------------------------------------

add_library(webservices_src OBJECT ${libwebservices_SRCS}
                                   ${libwso2_SRCS}
                                   ${libwsflickr_SRCS}
                                   ${libwsimgur_SRCS}
                                   ${libwsdropbox_SRCS}
                                   ${libwssmugmug_SRCS}
                                   ${libwsimageshack_SRCS}
                                   ${libwsfacebook_SRCS}
                                   ${libwsrajce_SRCS}
                                   ${libwsgoogle_SRCS}
                                   ${libwsyandexfotki_SRCS}
                                   ${libwspiwigo_SRCS}
                                   ${libwsmediawiki_SRCS}
                                   ${libwsvkontakte_SRCS}
                                   ${libwsfiletransfer_SRCS}
)
