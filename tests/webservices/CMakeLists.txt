#
# Copyright (c) 2010-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# =======================================================
# Stand alone version of ExpoBlending tool

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories(
    $<TARGET_PROPERTY:Qt5::Xml,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

# =======================================================
# Stand alone version of Flickr tool

set(flickr_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/flickr.cpp)
add_executable(flickr ${flickr_SRCS})

target_link_libraries(flickr
                      digikamcore

                      Qt5::Gui
                      Qt5::Xml
)

# =======================================================
# Stand alone version of Imgur tool

set(imgur_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/imgur.cpp)
add_executable(imgur ${imgur_SRCS})

target_link_libraries(imgur
                      digikamcore

                      Qt5::Gui
                      Qt5::Xml
)

# =======================================================
# Stand alone version of Dropbox tool

set(dropbox_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/dropbox.cpp)
add_executable(dropbox ${dropbox_SRCS})

target_link_libraries(dropbox
                      digikamcore

                      Qt5::Gui
                      Qt5::Xml
)

# =======================================================
# Stand alone version of SmugMug tool

set(smugmug_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/smugmug.cpp)
add_executable(smugmug ${smugmug_SRCS})

target_link_libraries(smugmug
                      digikamcore

                      Qt5::Gui
                      Qt5::Xml
)

# =======================================================
# Stand alone version of ImageShack tool

set(imageshack_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/imageshack.cpp)
add_executable(imageshack ${imageshack_SRCS})

target_link_libraries(imageshack
                      digikamcore

                      Qt5::Gui
                      Qt5::Xml
)

# =======================================================
# Stand alone version of FaceBook tool

set(facebook_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/facebook.cpp)
add_executable(facebook ${facebook_SRCS})

target_link_libraries(facebook
                      digikamcore

                      Qt5::Gui
                      Qt5::Xml
)

# =======================================================
# Stand alone version of Rajce tool

set(rajce_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/rajce.cpp)
add_executable(rajce ${rajce_SRCS})

target_link_libraries(rajce
                      digikamcore

                      Qt5::Gui
                      Qt5::Xml
)

# =======================================================
# Stand alone version of GDrive tool

set(gdrive_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/gdrive.cpp)
add_executable(gdrive ${gdrive_SRCS})

target_link_libraries(gdrive
                      digikamcore

                      Qt5::Gui
                      Qt5::Xml
)

# =======================================================
# Stand alone version of GPhoto tool

set(gphoto_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/gphoto.cpp)
add_executable(gphoto ${gphoto_SRCS})

target_link_libraries(gphoto
                      digikamcore

                      Qt5::Gui
                      Qt5::Xml
)

# =======================================================
# Stand alone version of YandexFotki tool

set(yandexfotki_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/yandexfotki.cpp)
add_executable(yandexfotki ${yandexfotki_SRCS})

target_link_libraries(yandexfotki
                      digikamcore

                      Qt5::Gui
                      Qt5::Xml
)

# =======================================================
# Stand alone version of Piwigo tool

set(piwigo_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/piwigo.cpp)
add_executable(piwigo ${piwigo_SRCS})

target_link_libraries(piwigo
                      digikamcore

                      Qt5::Gui
                      Qt5::Xml
)

# =======================================================
# Stand alone version of MediaWiki tool

if(KF5MediaWiki_FOUND)

    set(mediawiki_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/mediawiki.cpp)
    add_executable(mediawiki ${mediawiki_SRCS})

    target_link_libraries(mediawiki
                          digikamcore

                          Qt5::Gui
                          Qt5::Xml
    )

endif()

# =======================================================
# Stand alone version of VKontakte tool

if(KF5Vkontakte_FOUND)

    set(vkontakte_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/vkontakte.cpp)
    add_executable(vkontakte ${vkontakte_SRCS})

    target_link_libraries(vkontakte
                          digikamcore

                          Qt5::Gui
                          Qt5::Xml
    )

endif()

# =======================================================
# Stand alone version of FileTransfer tool

if(KF5KIO_FOUND)

    set(filetransfer_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/filetransfer.cpp)
    add_executable(filetransfer ${filetransfer_SRCS})

    target_link_libraries(filetransfer
                          digikamcore

                          Qt5::Gui
                          Qt5::Xml
    )

endif()

# =======================================================
# Stand alone version of factored WebServices tool

set(webservices_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/webservices.cpp)

add_executable(webservices ${webservices_SRCS})

target_link_libraries(webservices
                      digikamcore

                      Qt5::Gui
                      Qt5::Xml
)
