
ADD_SUBDIRECTORY( pics )
ADD_SUBDIRECTORY( icons )
ADD_SUBDIRECTORY( setup )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/showfoto/setup 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/thumbbar 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/metadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/jpegutils 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/greycstoration 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/imageproperties 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/loaders 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/curves 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/levels 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/lprof 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/whitebalance 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/histogram 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/threadimageio 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/setup 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/tools 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/slideshow 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/splashscreen 
                     ${CMAKE_SOURCE_DIR}/digikam/digikam 
                   )

SET(showfoto_SRCS 
    ${libshowfotosetup_SRCS} 
    ${libdimgcanvas_SRCS}
    ${libsetupshowfoto_SRCS} 
    ${libsplashscreen_SRCS} 
    ${libthumbbar_SRCS} 
    ${libjpegutils_SRCS} 
    ${libcommonwidgets_SRCS} 
    ${libthreadimageio_SRCS} 
    ${libimagepluginswidgets_SRCS} 
    ${libshowfoto_SRCS} 
    ${libdimgeditortools_SRCS}
    ${libslideshow_SRCS} 
    main.cpp 
    showfoto.cpp 
   )

KDE4_AUTOMOC(${showfoto_SRCS})

KDE4_ADD_EXECUTABLE(showfoto ${showfoto_SRCS})

TARGET_LINK_LIBRARIES(showfoto libshowfotosetup
                               libsetupshowfoto
                               libdialogshowfoto 
                               libimagepropertiesshowfoto 
                               libiccprofileswidgets
                               libmetadatawidgets
                               libdimgcanvas
                               libshowfoto
                               libdimgeditortools
                               libslideshow
                               ${KDE4_KUTILS_LIBS} 
                               ${KDE4_KPARTS_LIBS} 
                     )

INSTALL(TARGETS showfoto DESTINATION ${BIN_INSTALL_DIR})

INSTALL( FILES showfoto.desktop DESTINATION ${XDG_APPS_DIR} )
INSTALL( FILES showfotoui.rc DESTINATION ${DATA_INSTALL_DIR}/showfoto )
