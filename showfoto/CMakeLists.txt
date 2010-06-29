SET_DEBUG_AREA_CODE(${AREA_CODE_SHOWFOTO})

#Require by Greycstoration algorithm (CImg.h)
IF(NOT MSVC)
    ADD_DEFINITIONS(${KDE4_ENABLE_EXCEPTIONS})
ENDIF(NOT MSVC)

INCLUDE_DIRECTORIES( ../showfoto/setup
                     ../libs/widgets/common
                     ../libs/widgets/metadata
                     ../libs/widgets/iccprofiles
                     ../libs/jpegutils
                     ../libs/imageproperties
                     ../libs/dimg
                     ../libs/dimg/loaders
                     ../libs/dimg/filters
                     ../libs/dimg/filters/auto
                     ../libs/dimg/filters/bcg
                     ../libs/dimg/filters/bw
                     ../libs/dimg/filters/decorate
                     ../libs/dimg/filters/fx
                     ../libs/dimg/filters/greycstoration
                     ../libs/dimg/filters/hsl
                     ../libs/dimg/filters/icc
                     ../libs/dimg/filters/lc
                     ../libs/dimg/filters/nr
                     ../libs/dimg/filters/sharp
                     ../libs/dimg/filters/levels
                     ../libs/dimg/filters/curves
                     ../libs/dimg/filters/wb
                     ../libs/dimg/filters/transform
                     ../libs/dimg/filters/lens
                     ../libs/dmetadata
                     ../libs/themeengine
                     ../libs/histogram
                     ../libs/threadimageio
                     ../libs/threads
                     ../libs/dialogs
                     ../libs/versionmanager
                     ../utilities/setup
                     ../utilities/imageeditor/canvas
                     ../utilities/imageeditor/widgets
                     ../utilities/imageeditor/editor
                     ../utilities/imageeditor/printiface
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

KDE4_ADD_APP_ICON(showfoto_SRCS "../data/icons/apps/hi*-app-showfoto.png")

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
