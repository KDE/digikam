#Require by Greycstoration algorithm (CImg.h)
ADD_DEFINITIONS(${KDE4_ENABLE_EXCEPTIONS})

ADD_SUBDIRECTORY(setup)

INCLUDE_DIRECTORIES( ../showfoto/setup 
                     ../libs/widgets/common 
                     ../libs/widgets/imageplugins 
                     ../libs/widgets/metadata 
                     ../libs/widgets/iccprofiles 
                     ../libs/jpegutils 
                     ../libs/greycstoration 
                     ../libs/imageproperties 
                     ../libs/dimg 
                     ../libs/dimg/filters 
                     ../libs/dimg/loaders 
                     ../libs/dmetadata 
                     ../libs/themeengine 
                     ../libs/curves 
                     ../libs/levels 
                     ../libs/lprof 
                     ../libs/whitebalance 
                     ../libs/histogram 
                     ../libs/threadimageio 
                     ../libs/dialogs 
                     ../utilities/setup 
                     ../utilities/imageeditor/canvas 
                     ../utilities/imageeditor/editor 
                     ../utilities/imageeditor/tools 
                     ../utilities/slideshow 
                     ../digikam 

                     # version.h
                     ${CMAKE_CURRENT_BINARY_DIR}/../digikam
                     # external library paths
                     ${LCMS_INCLUDE_DIR}
                   )

SET(showfoto_SRCS

    ${libsetupshowfoto_SRCS}
    ${libshowfotosetup_SRCS}

    main.cpp
    showfoto.cpp 
   )


KDE4_ADD_EXECUTABLE(showfoto ${showfoto_SRCS})

TARGET_LINK_LIBRARIES(showfoto digikam)

INSTALL(TARGETS showfoto DESTINATION ${BIN_INSTALL_DIR})

INSTALL(FILES showfoto.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
INSTALL(FILES showfotoui.rc DESTINATION ${DATA_INSTALL_DIR}/showfoto )
