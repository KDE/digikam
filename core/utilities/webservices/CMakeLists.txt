#
# Copyright (c) 2010-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
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

if(ENABLE_QWEBENGINE)
    include_directories($<TARGET_PROPERTY:Qt5::WebEngineWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
else()
    include_directories($<TARGET_PROPERTY:Qt5::WebKitWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

if(KF5KIO_FOUND)
    include_directories($<TARGET_PROPERTY:KF5::KIOCore,INTERFACE_INCLUDE_DIRECTORIES>
                        $<TARGET_PROPERTY:KF5::KIOFileWidgets,INTERFACE_INCLUDE_DIRECTORIES>
    )
endif()

# Helper classes ------------------------------------------------------------------

set(libwebservices_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/common/wsstarter.cpp
                        #${CMAKE_CURRENT_SOURCE_DIR}/common/manager/wsauthentication.cpp
                        #${CMAKE_CURRENT_SOURCE_DIR}/common/manager/wstalker.cpp
                        #${CMAKE_CURRENT_SOURCE_DIR}/common/wizard/wsalbumspage.cpp
                        #${CMAKE_CURRENT_SOURCE_DIR}/common/wizard/wsfinalpage.cpp
                        #${CMAKE_CURRENT_SOURCE_DIR}/common/wizard/wsimagespage.cpp
                        #${CMAKE_CURRENT_SOURCE_DIR}/common/wizard/wsintropage.cpp
                        #${CMAKE_CURRENT_SOURCE_DIR}/common/wizard/wssettingspage.cpp
                        #${CMAKE_CURRENT_SOURCE_DIR}/common/wizard/wsauthenticationpage.cpp
                        #${CMAKE_CURRENT_SOURCE_DIR}/common/wizard/wswizard.cpp
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

# SmugMug tool -------------------------------------------------------------

set(libwssmugmug_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/smugmug/smugnewalbumdlg.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/smugmug/smugmpform.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/smugmug/smugtalker.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/smugmug/smugwidget.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/smugmug/smugwindow.cpp
)

# FaceBook tool -------------------------------------------------------------

set(libwsfacebook_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/facebook/fbnewalbumdlg.cpp
                       ${CMAKE_CURRENT_SOURCE_DIR}/facebook/fbmpform.cpp
                       ${CMAKE_CURRENT_SOURCE_DIR}/facebook/fbtalker.cpp
                       ${CMAKE_CURRENT_SOURCE_DIR}/facebook/fbwidget.cpp
                       ${CMAKE_CURRENT_SOURCE_DIR}/facebook/fbwindow.cpp
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

set(libwsyandexfotki_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/yandexfotki/yfauth/yfauth.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/yandexfotki/yfauth/yfrsa.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/yandexfotki/yfalbum.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/yandexfotki/yftalker.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/yandexfotki/yfphoto.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/yandexfotki/yfnewalbumdlg.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/yandexfotki/yfwindow.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/yandexfotki/yfwidget.cpp
)

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
                                   ${libwsipfs_SRCS}
                                   ${libwsdropbox_SRCS}
                                   ${libwsonedrive_SRCS}
                                   ${libwspinterest_SRCS}
                                   ${libwsbox_SRCS}
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
