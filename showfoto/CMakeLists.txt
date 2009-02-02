#Require by Greycstoration algorithm (CImg.h)
IF(NOT MSVC)
    ADD_DEFINITIONS(${KDE4_ENABLE_EXCEPTIONS})
ENDIF(NOT MSVC)

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
                     ../utilities/imageeditor/rawimport 
                     ../utilities/slideshow 
                     ../digikam 
                     ../

                     # version.h
                     ${CMAKE_CURRENT_BINARY_DIR}/../
                     # external library paths
                     ${LCMS_INCLUDE_DIR}
                   )

SET(showfoto_SRCS

    ${libsetupshowfoto_SRCS}
    ${libshowfotosetup_SRCS}

    main.cpp
    showfoto.cpp 
   )

KDE4_ADD_APP_ICON(showfoto_SRCS "../data/icons/oxygen/*/apps/showfoto.png")

KDE4_ADD_EXECUTABLE(showfoto ${showfoto_SRCS})

TARGET_LINK_LIBRARIES(showfoto 
                      digikamcore 
                      ${KEXIV2_LIBRARIES} 
                      ${KDCRAW_LIBRARIES} 
                      ${KDE4_KIO_LIBS} 
                      ${LCMS_LIBRARIES}
                      ${QT_QT3SUPPORT_LIBRARY}
                     )

INSTALL(TARGETS showfoto ${INSTALL_TARGETS_DEFAULT_ARGS})

INSTALL(FILES showfoto.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
INSTALL(FILES showfotoui.rc DESTINATION ${DATA_INSTALL_DIR}/showfoto )
