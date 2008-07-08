IF(NOT WIN32)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)
ENDIF(NOT WIN32)

#Require by Greycstoration algorithm (CImg.h)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

INCLUDE_DIRECTORIES(
                     # basic libs
                     ../libs/dimg
                     ../libs/dimg/loaders
                     ../libs/dimg/filters
                     ../libs/whitebalance
                     ../libs/dmetadata 
                     ../libs/histogram
                     ../libs/curves
                     ../libs/levels
                     ../libs/lprof/
                     ../libs/jpegutils 
                     ../libs/greycstoration
                     ../libs/threadimageio

                     # widgets and dialogs
                     ../libs/widgets/common
                     ../libs/widgets/imageplugins 
                     ../libs/widgets/metadata 
                     ../libs/widgets/iccprofiles 
                     ../libs/imageproperties
                     ../libs/dialogs

                     # database libs
                     ../libs/database
                     ../libs/database/sqlite2
                     ../libs/database/sqlite3
                     ../libs/database/haar

                     # shared utilities
                     ../utilities/slideshow
                     ../utilities/imageeditor/editor
                     ../utilities/imageeditor/canvas
                     ../utilities/imageeditor/tools

                     # specific libs and utilities
                     ../libs/themeengine
                     ../utilities/kipiiface
                     ../utilities/cameragui
                     ../utilities/setup
                     ../utilities/slideshow
                     ../utilities/batch
                     ../utilities/lighttable
                     ../utilities/searchwindow
                     ../utilities/fuzzysearch
                     ../utilities/gpssearch
                     ../utilities/timeline
                     ../digikam
                     ../

                     # config-digikam.h
                     ${CMAKE_CURRENT_BINARY_DIR}/../
                     # external library paths
                     ${JPEG_INCLUDE_DIS}
                     ${PNG_PNG_INCLUDE_DIR}
                     ${TIFF_INCLUDE_DIR}
                   )


# Shared libdigikam ################################################################

SET(digikam_LIB_SRCS
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
   )

KDE4_ADD_LIBRARY(digikam SHARED ${digikam_LIB_SRCS})

TARGET_LINK_LIBRARIES(digikam
                      ${KDE4_KDECORE_LIBS} 
                      ${KDE4_KIO_LIBS} 
                      ${KDE4_KHTML_LIBS} 
                      ${KDE4_KDEUI_LIBS} 
                      ${KDE4_KUTILS_LIBS} 
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
  TARGET_LINK_LIBRARIES(digikam ${LIBMARBLEWIDGET_LIBRARY})
ENDIF(MARBLEWIDGET_FOUND)

SET_TARGET_PROPERTIES(digikam PROPERTIES VERSION 1.0.0 SOVERSION 1 )
INSTALL(TARGETS digikam ${INSTALL_TARGETS_DEFAULT_ARGS} )


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
                      digikam
                      ${QT_QTSQL_LIBRARY}
                     )

SET_TARGET_PROPERTIES(digikamdatabase PROPERTIES VERSION 1.0.0 SOVERSION 1 )
INSTALL(TARGETS digikamdatabase ${INSTALL_TARGETS_DEFAULT_ARGS} )


# digiKam binary progam #########################################################

SET(digikam_bin_SRCS
                     ${libdimgimagewindow_SRCS}
                     ${libdeletedialog_SRCS}
                     ${libimagepropertiesdigikam_SRCS}
                     ${libsetup_SRCS}
                     ${libcameragui_SRCS}
                     ${liblighttable_SRCS}
                     ${libbatch_SRCS}
                     ${libthemeengine_SRCS}
                     ${libkipiiface_SRCS}
                     ${libsearchwindow_SRCS}
                     ${libfuzzysearch_SRCS}
                     ${libgpssearch_SRCS}
                     ${libtimeline_SRCS}

                     album.cpp 
                     albumiconview.cpp 
                     albumiconviewfilter.cpp 
                     albumicongroupitem.cpp 
                     albumiconitem.cpp 
                     albumitemhandler.cpp 
                     albumfiletip.cpp 
                     albumfolderview.cpp 
                     albumhistory.cpp 
                     albumlister.cpp 
                     albummanager.cpp 
                     albummodel.cpp
                     albumpropsedit.cpp 
                     albumselectdialog.cpp
                     albumsettings.cpp 
                     albumthumbnailloader.cpp 
                     albumwidgetstack.cpp 
                     cameralist.cpp 
                     cameratype.cpp 
                     datefolderview.cpp 
                     digikamapp.cpp 
                     digikamfirstrun.cpp 
                     digikamview.cpp 
                     dio.cpp 
                     ddragobjects.cpp 
                     folderitem.cpp 
                     folderview.cpp 
                     iconview.cpp 
                     icongroupitem.cpp 
                     iconitem.cpp 
                     imageattributeswatch.cpp 
                     imagepreviewview.cpp 
                     imagepreviewbar.cpp 
                     kdateedit.cpp 
                     kdatetimeedit.cpp 
                     kdatepickerpopup.cpp 
                     main.cpp
                     mediaplayerview.cpp 
                     metadatahub.cpp 
                     mimefilter.cpp 
                     monthwidget.cpp 
                     ratingfilter.cpp
                     ratingpopupmenu.cpp 
                     ratingwidget.cpp 
                     scancontroller.cpp
                     syncjob.cpp 
                     tageditdlg.cpp 
                     tagfolderview.cpp 
                     tagfilterview.cpp 
                     treefolderitem.cpp 
                     tagspopupmenu.cpp 
                     welcomepageview.cpp 
    )

QT4_ADD_DBUS_ADAPTOR(digikam_bin_SRCS org.kde.digikam.xml digikamapp.h Digikam::DigikamApp)

KDE4_ADD_EXECUTABLE(digikam_bin ${digikam_bin_SRCS})

TARGET_LINK_LIBRARIES(digikam_bin
                      ${KDE4_KPARTS_LIBS}
                      ${KDE4_PHONON_LIBS}
                      ${STRIGI_LIBRARIES}
                      ${GPHOTO2_LIBRARIES}
                      digikam
                      digikamdatabase
                     )

IF(KDEPIMLIBS_FOUND)
  TARGET_LINK_LIBRARIES(digikam_bin ${KDE4_KABC_LIBS})
ENDIF(KDEPIMLIBS_FOUND)

SET_TARGET_PROPERTIES(digikam_bin PROPERTIES OUTPUT_NAME digikam)

INSTALL(TARGETS digikam_bin ${INSTALL_TARGETS_DEFAULT_ARGS})


# Others Files to install #######################################################

INSTALL(FILES digikam.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
INSTALL(FILES digikamui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikam_export.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
