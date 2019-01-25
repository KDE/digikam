#
# Copyright (c) 2010-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(ENABLE_QWEBENGINE)
    include_directories($<TARGET_PROPERTY:Qt5::WebEngineWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
else()
    include_directories($<TARGET_PROPERTY:Qt5::WebKitWidgets,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

add_subdirectory(box)
add_subdirectory(dropbox)
add_subdirectory(facebook)
add_subdirectory(flickr)
add_subdirectory(google)
add_subdirectory(imgur)
add_subdirectory(imageshack)
add_subdirectory(ipfs)
add_subdirectory(mediawiki)
add_subdirectory(onedrive)
add_subdirectory(pinterest)
add_subdirectory(piwigo)
add_subdirectory(rajce)
add_subdirectory(smugmug)
add_subdirectory(yandexfotki)

if(KF5Vkontakte_FOUND)
    add_subdirectory(vkontakte)
endif()

if(KF5KIO_FOUND)
    add_subdirectory(filetransfer)

#    Not yet finalized
#    add_subdirectory(debianscreenshot)
endif()

# Not yet finalized
#add_subdirectory(twitter)
#add_subdirectory(unified)
