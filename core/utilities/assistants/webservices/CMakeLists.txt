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

set(libwebservices_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/common/wsnewalbumdialog.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/common/wsstarter.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/common/manager/wssettings.cpp
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

# Imgur tool ----------------------------------------------------------------

set(libwsimgur_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/imgur/imgurtalker.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/imgur/imgurimageslist.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/imgur/imgurwindow.cpp
)

# Ipfs tool -----------------------------------------------------------------

set(libwsipfs_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/ipfs/ipfstalker.cpp
                   ${CMAKE_CURRENT_SOURCE_DIR}/ipfs/ipfsimageslist.cpp
                   ${CMAKE_CURRENT_SOURCE_DIR}/ipfs/ipfswindow.cpp
)

# DropBox tool --------------------------------------------------------------

set(libwsdropbox_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/dropbox/dbnewalbumdlg.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/dropbox/dbmpform.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/dropbox/dbtalker.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/dropbox/dbwidget.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/dropbox/dbwindow.cpp
)

# OneDrive tool --------------------------------------------------------------

set(libwsonedrive_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/onedrive/odnewalbumdlg.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/onedrive/odmpform.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/onedrive/odtalker.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/onedrive/odwidget.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/onedrive/odwindow.cpp
)

# Pinterest tool --------------------------------------------------------------

set(libwspinterest_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/pinterest/pnewalbumdlg.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/pinterest/ptalker.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/pinterest/pwidget.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/pinterest/pwindow.cpp
)

# Box tool --------------------------------------------------------------

set(libwsbox_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/box/boxnewalbumdlg.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/box/boxtalker.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/box/boxwidget.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/box/boxwindow.cpp
)


# Twitter tool --------------------------------------------------------------

set(libwstwitter_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/twitter/twitternewalbumdlg.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/twitter/twittermpform.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/twitter/twittertalker.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/twitter/twitterwidget.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/twitter/twitterwindow.cpp
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

set(libwsyandexfotki_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/yandexfotki/yfauth/yfauth.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/yandexfotki/yfauth/yfrsa.cpp
                          ${CMAKE_CURRENT_SOURCE_DIR}/yandexfotki/yfalbum.cpp
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

set(libwsmediawiki_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/mediawikiwindow.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/mediawikiwidget.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/mediawikitalker.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/iface/mediawiki_image.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/iface/mediawiki_imageinfo.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/iface/mediawiki_iface.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/iface/mediawiki_login.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/iface/mediawiki_logout.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/iface/mediawiki_queryimageinfo.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/iface/mediawiki_queryimages.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/iface/mediawiki_querysiteinfogeneral.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/iface/mediawiki_generalinfo.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/iface/mediawiki_querysiteinfousergroups.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/iface/mediawiki_queryrevision.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/iface/mediawiki_queryinfo.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/iface/mediawiki_edit.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/iface/mediawiki_job.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/iface/mediawiki_usergroup.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/iface/mediawiki_revision.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/iface/mediawiki_page.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/iface/mediawiki_parse.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/iface/mediawiki_protection.cpp
                        ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki/iface/mediawiki_upload.cpp
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
                                   ${libwsimgur_SRCS}
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

                                   # Not yet finalized
                                   #${libwstwitter_SRCS}
)
