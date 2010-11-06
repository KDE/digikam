SET_DEBUG_AREA_CODE(${AREA_CODE_SHOWFOTO})

#Require by Greycstoration algorithm (CImg.h)
IF(NOT MSVC)
    ADD_DEFINITIONS(${KDE4_ENABLE_EXCEPTIONS})
ENDIF(NOT MSVC)

INCLUDE_DIRECTORIES( 
                     ${CMAKE_CURRENT_SOURCE_DIR}/../showfoto/setup
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/widgets/common
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/widgets/metadata
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/widgets/iccprofiles
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/jpegutils
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/imageproperties
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/loaders
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters/auto
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters/bcg
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters/bw
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters/decorate
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters/fx
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters/greycstoration
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters/hsl
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters/icc
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters/lc
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters/nr
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters/sharp
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters/levels
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters/curves
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters/wb
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters/transform
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters/lens
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dmetadata
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/themeengine
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/threadimageio
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/threads
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dialogs
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/canvas
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/widgets
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/editor
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/printiface
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/rawimport
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/slideshow
                     ${CMAKE_CURRENT_SOURCE_DIR}/../digikam
                     ${CMAKE_CURRENT_SOURCE_DIR}/../

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

IF(CLAPACK_FOUND)
   TARGET_LINK_LIBRARIES(showfoto ${CLAPACK_LIBRARY})
ENDIF(CLAPACK_FOUND)

TARGET_LINK_LIBRARIES(showfoto
                      digikamcore
                      ${KEXIV2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
                      ${KDE4_KIO_LIBS}
                      ${LCMS_LIBRARIES}
                      ${QT_QT3SUPPORT_LIBRARY}
                     )

INSTALL(TARGETS showfoto ${INSTALL_TARGETS_DEFAULT_ARGS})

INSTALL(PROGRAMS showfoto.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
INSTALL(FILES showfotoui.rc DESTINATION ${DATA_INSTALL_DIR}/showfoto )
