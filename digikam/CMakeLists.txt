CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)

INCLUDE_DIRECTORIES(
                     # basic libs
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/loaders
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/whitebalance
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dmetadata 
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/histogram
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/curves
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/levels
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/lprof
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/jpegutils 
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/greycstoration
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/threadimageio

                     # widgets and dialogs
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/widgets/common
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/widgets/imageplugins 
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/widgets/metadata 
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/widgets/iccprofiles 
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/imageproperties
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dialogs

                     # database libs
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/database
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/database/sqlite2
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/database/haar

                     # shared utilities
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/slideshow
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/editor
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/canvas
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/tools
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/rawimport

                     # specific libs and utilities
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/themeengine
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/kipiiface
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/cameragui
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/setup
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/slideshow
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/batch
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/lighttable
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/searchwindow
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/fuzzysearch
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/gpssearch
                     ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/timeline
                     ${CMAKE_CURRENT_SOURCE_DIR}/../digikam

                     # config-digikam.h
                     ${CMAKE_CURRENT_BINARY_DIR}/../

                     # external library paths
                     ${JPEG_INCLUDE_DIS}
                     ${PNG_PNG_INCLUDE_DIR}
                     ${TIFF_INCLUDE_DIR}
                   )


# Shared libdigikamcore ################################################################

SET(digikamcore_LIB_SRCS
                        # basic libs
                        ${libdimg_SRCS}
                        ${libdimgloaders_SRCS}
                        ${libdimgfilters_SRCS}
                        ${libdmetadata_SRCS}
                        ${libhistogram_SRCS}
                        ${libwhitebalance_SRCS}
                        ${libcurves_SRCS}
                        ${liblevels_SRCS}
                        ${liblprof_SRCS}
                        ${libjpegutils_SRCS}
                        ${libgreycstoration_SRCS}
                        ${libthreadimageio_SRCS}
                        ${libthemeengine_SRCS}

                        # widgets and dialogs
                        ${libcommonwidgets_SRCS}
                        ${libimagepluginswidgets_SRCS}
                        ${libiccprofileswidgets_SRCS}
                        ${libmetadatawidgets_SRCS}
                        ${libdialogs_SRCS}
                        ${libimageproperties_SRCS}

                        # utitilies
                        ${libslideshow_SRCS}
                        ${libdimgcanvas_SRCS}
                        ${libdimgeditortools_SRCS}
                        ${libdimgeditor_SRCS}
                        ${librawimport_SRCS}
   )

KDE4_ADD_LIBRARY(digikamcore SHARED ${digikamcore_LIB_SRCS})

TARGET_LINK_LIBRARIES(digikamcore
                      ${KDE4_KDECORE_LIBS} 
                      ${KDE4_KIO_LIBS} 
                      ${KDE4_KHTML_LIBS} 
                      ${KDE4_KDEUI_LIBS} 
                      ${KDE4_KUTILS_LIBS} 
                      ${KDE4_SOLID_LIBS}
                      ${KDE4_KDE3SUPPORT_LIBRARY}
                      ${QT_QT3SUPPORT_LIBRARY}

                      ${JPEG_LIBRARIES}
                      ${TIFF_LIBRARIES}
                      ${PNG_LIBRARIES}
                      ${LCMS_LIBRARIES}
                      ${JASPER_LIBRARIES}

                      # Require by CImg library which use pthread internally.
                      ${CMAKE_THREAD_LIBS_INIT} 

                      ${KDCRAW_LIBRARIES}
                      ${KEXIV2_LIBRARIES}
                      ${KIPI_LIBRARIES}
              )

IF(MARBLEWIDGET_FOUND)
    TARGET_LINK_LIBRARIES(digikamcore ${LIBMARBLEWIDGET_LIBRARY})
ENDIF(MARBLEWIDGET_FOUND)

SET_TARGET_PROPERTIES(digikamcore PROPERTIES VERSION 1.0.0 SOVERSION 1)
IF(WIN32)
    SET_TARGET_PROPERTIES(digikamcore PROPERTIES COMPILE_FLAGS -DJPEG_STATIC)
    SET_TARGET_PROPERTIES(digikamcore PROPERTIES OUTPUT_NAME digikamlib)
ENDIF(WIN32)
INSTALL(TARGETS digikamcore ${INSTALL_TARGETS_DEFAULT_ARGS})


# Shared libdigikamdatabase ########################################################

# Disable libsqlite2 C code warnings.
FOREACH(_curentfile ${libsqlite2_SRCS})
    SET_SOURCE_FILES_PROPERTIES(${_curentfile} PROPERTIES COMPILE_FLAGS -w)
ENDFOREACH(_curentfile ${libsqlite2_SRCS})

SET(digikamdatabase_LIB_SRCS ${libdatabase_SRCS}
                             ${libhaar_SRCS}
                             ${libsqlite2_SRCS}
   )

KDE4_ADD_LIBRARY(digikamdatabase SHARED ${digikamdatabase_LIB_SRCS})

TARGET_LINK_LIBRARIES(digikamdatabase
                      digikamcore
                      ${KDE4_KIO_LIBS}
                      ${KDE4_SOLID_LIBS}
                      ${KEXIV2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTSQL_LIBRARY}
                     )

SET_TARGET_PROPERTIES(digikamdatabase PROPERTIES VERSION 1.0.0 SOVERSION 1 )
IF(WIN32)
    SET_SOURCE_FILES_PROPERTIES(${libsqlite2_SRCS} PROPERTIES COMPILE_FLAGS "-D_OFF_T_ -D_OFF_T_DEFINED")
ENDIF(WIN32)
INSTALL(TARGETS digikamdatabase ${INSTALL_TARGETS_DEFAULT_ARGS} )


# digiKam binary progam #########################################################

SET(digikam_SRCS ${libdimgimagewindow_SRCS}
                 ${libdeletedialog_SRCS}
                 ${libimagepropertiesdigikam_SRCS}
                 ${libsetup_SRCS}
                 ${libcameragui_SRCS}
                 ${liblighttable_SRCS}
                 ${libbatch_SRCS}
                 ${libkipiiface_SRCS}
                 ${libsearchwindow_SRCS}
                 ${libfuzzysearch_SRCS}
                 ${libgpssearch_SRCS}
                 ${libtimeline_SRCS}
                 ${libdigikamgui_SRCS}

                 main.cpp
    )

QT4_ADD_DBUS_ADAPTOR(digikam_SRCS org.kde.digikam.xml digikamapp.h Digikam::DigikamApp)

KDE4_ADD_APP_ICON(digikam_SRCS "../data/icons/hi*-app-digikam.png")

KDE4_ADD_EXECUTABLE(digikam ${digikam_SRCS})

TARGET_LINK_LIBRARIES(digikam
                      ${KDE4_KPARTS_LIBS}
                      ${KDE4_PHONON_LIBS}
                      ${KDE4_KHTML_LIBS}
                      ${KDE4_KUTILS_LIBS}
                      ${KDE4_SOLID_LIBS}
                      ${KIPI_LIBRARIES}
                      ${KEXIV2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
                      ${QT_QTSQL_LIBRARY}
                      ${QT_QT3SUPPORT_LIBRARY}
                      ${STRIGI_LIBRARIES}
                      ${LCMS_LIBRARIES}
                      ${GPHOTO2_LIBRARIES}
                      digikamcore
                      digikamdatabase
                     )

IF(KDEPIMLIBS_FOUND)
  TARGET_LINK_LIBRARIES(digikam ${KDE4_KABC_LIBS})
ENDIF(KDEPIMLIBS_FOUND)

IF(MARBLEWIDGET_FOUND)
  TARGET_LINK_LIBRARIES(digikam ${LIBMARBLEWIDGET_LIBRARY})
ENDIF(MARBLEWIDGET_FOUND)

INSTALL(TARGETS digikam ${INSTALL_TARGETS_DEFAULT_ARGS})


# Others Files to install #######################################################

INSTALL(FILES digikam.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
INSTALL(FILES digikamui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikam_export.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
