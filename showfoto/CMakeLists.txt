#Require by Greycstoration algorithm (CImg.h)
ADD_DEFINITIONS(${KDE4_ENABLE_EXCEPTIONS})

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
                     ${CMAKE_SOURCE_DIR}/digikam/digikam 
                   )

SET(showfoto_SRCS 
    ${libslideshow_SRCS} 
    ${libshowfotosetup_SRCS} 
    ${libsetupshowfoto_SRCS} 
    ${libdimgcanvas_SRCS}
    ${libdimgeditortools_SRCS}
    ${libshowfoto_SRCS} 

    ${libdimg_SRCS}
    ${libdimgloaders_SRCS}
    ${libdimgfilters_SRCS}
    ${libdmetadata_SRCS}
    ${libthumbbar_SRCS} 
    ${libhistogram_SRCS}
    ${libwhitebalance_SRCS}
    ${libcurves_SRCS}
    ${liblevels_SRCS}
    ${liblprof_SRCS}
    ${libjpegutils_SRCS} 
    ${libgreycstoration_SRCS}
    ${libthreadimageio_SRCS} 
    ${libcommonwidgets_SRCS} 
    ${libimagepluginswidgets_SRCS} 
    ${libimagepropertiesshowfoto_SRCS}
    ${libiccprofileswidgets_SRCS}
    ${libmetadatawidgets_SRCS}
    ${libdialogshowfoto_SRCS}

    main.cpp 
    showfoto.cpp 
   )


KDE4_ADD_EXECUTABLE(showfoto ${showfoto_SRCS})

TARGET_LINK_LIBRARIES(showfoto ${KDE4_KUTILS_LIBS}
                               ${KDE4_KDEPRINT_LIBS}  
                               ${KDE4_KPARTS_LIBS} 
                               ${KDE4_KDE3SUPPORT_LIBS} 
                               ${KDE4_KIO_LIBS} 
                               ${QT_QT3SUPPORT_LIBRARY} 
                               ${KEXIV2_LIBRARIES} 
                               ${JPEG_LIBRARIES} 
                               ${TIFF_LIBRARIES} 
                               ${PNG_LIBRARIES} 
                               ${JASPER_LIBRARIES} 
                               ${KDCRAW_LIBRARIES} 
                               ${LCMS_LIBRARIES} 
                               ${KEXIV2_LIBRARIES} 
                     )

INSTALL(TARGETS showfoto DESTINATION ${BIN_INSTALL_DIR})

INSTALL( FILES showfoto.desktop DESTINATION ${XDG_APPS_DIR} )
INSTALL( FILES showfotoui.rc DESTINATION ${DATA_INSTALL_DIR}/showfoto )
