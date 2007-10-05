IF(NOT WIN32)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)
ENDIF(NOT WIN32)

#Require by Greycstoration algorithm (CImg.h)
ADD_DEFINITIONS(${KDE4_ENABLE_EXCEPTIONS})

INCLUDE_DIRECTORIES(
                     # basic libs
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/loaders
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters
                     ${CMAKE_SOURCE_DIR}/digikam/libs/whitebalance
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/histogram
                     ${CMAKE_SOURCE_DIR}/digikam/libs/curves
                     ${CMAKE_SOURCE_DIR}/digikam/libs/levels
                     ${CMAKE_SOURCE_DIR}/digikam/libs/lprof/
                     ${CMAKE_SOURCE_DIR}/digikam/libs/jpegutils 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/greycstoration
                     ${CMAKE_SOURCE_DIR}/digikam/libs/threadimageio

                     # widgets and dialogs
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/metadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/imageproperties
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs

                     # database libs
                     ${CMAKE_SOURCE_DIR}/digikam/libs/database
                     ${CMAKE_SOURCE_DIR}/digikam/libs/database/sqlite2

                     # shared utilities
                     ${CMAKE_SOURCE_DIR}/digikam/libs/thumbbar
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/slideshow
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/tools

                     # specific libs and utilities
                     ${CMAKE_SOURCE_DIR}/digikam/libs/themeengine
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/cameragui
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/setup
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/slideshow
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/batch
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/lighttable
                     ${CMAKE_SOURCE_DIR}/digikam/digikam
                   )


# Shared libdigikam ################################################################

# Required by Greycstoration algorithm (CImg.h)
ADD_DEFINITIONS(${KDE4_ENABLE_EXCEPTIONS})

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

                        # widgets and dialogs
                        ${libcommonwidgets_SRCS}
                        ${libimagepluginswidgets_SRCS}
                        ${libiccprofileswidgets_SRCS}
                        ${libmetadatawidgets_SRCS}
                        ${libdialog_SRCS}
                        ${libimageproperties_SRCS}

                        # utitilies
                        ${libslideshow_SRCS}
                        ${libthumbbar_SRCS}
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

                      ${JPEG_LIBRARIES}
                      ${TIFF_LIBRARIES}
                      ${PNG_LIBRARIES}
                      ${LCMS_LIBRARIES}
                      ${JASPER_LIBRARIES}

                      ${KDCRAW_LIBRARIES}
                      ${KEXIV2_LIBRARIES}
                      ${KIPI_LIBRARIES}
              )

SET_TARGET_PROPERTIES(digikam PROPERTIES VERSION 1.0.0 SOVERSION 1 )
INSTALL(TARGETS digikam DESTINATION ${LIB_INSTALL_DIR} )


# Shared libdigikamdatabase ########################################################

SET(digikamdatabase_LIB_SRCS
                      ${libdatabase_SRCS}
                      ${libsqlite2_SRCS}
   )

KDE4_ADD_LIBRARY(digikamdatabase SHARED ${digikamdatabase_LIB_SRCS})

TARGET_LINK_LIBRARIES(digikamdatabase
                      digikam
                      ${QT_QTSQL_LIBRARY}
              )

SET_TARGET_PROPERTIES(digikamdatabase PROPERTIES VERSION 1.0.0 SOVERSION 1 )
INSTALL(TARGETS digikamdatabase DESTINATION ${LIB_INSTALL_DIR} )


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

                     album.cpp 
                     albumiconview.cpp 
                     albumicongroupitem.cpp 
                     albumiconitem.cpp 
                     albumitemhandler.cpp 
                     albumfiletip.cpp 
                     albumfolderview.cpp 
                     albumhistory.cpp 
                     albumlister.cpp 
                     albummanager.cpp 
                     albumpropsedit.cpp 
		     albumselectdialog.cpp
                     albumsettings.cpp 
                     albumthumbnailloader.cpp 
                     albumwidgetstack.cpp 
                     cameradragobject.cpp 
                     cameralist.cpp 
                     cameratype.cpp 
                     datefolderview.cpp 
                     digikamapp.cpp 
                     digikamfirstrun.cpp 
                     digikamview.cpp 
                     dio.cpp 
                     dragobjects.cpp 
                     firstrun.cpp 
                     folderitem.cpp 
                     folderview.cpp 
                     iconview.cpp 
                     icongroupitem.cpp 
                     iconitem.cpp 
                     imageattributeswatch.cpp 
                     imagepreviewview.cpp 
                     kdateedit.cpp 
                     kdatetimeedit.cpp 
                     kdatepickerpopup.cpp 
                     kipiinterface.cpp 
                     main.cpp
                     mediaplayerview.cpp 
                     metadatahub.cpp 
                     monthwidget.cpp 
                     pixmapmanager.cpp 
                     ratingpopupmenu.cpp 
                     ratingwidget.cpp 
                     scanlib.cpp 
                     searchadvanceddialog.cpp 
                     searchfolderview.cpp 
                     searchquickdialog.cpp 
                     searchresultsview.cpp 
                     searchresultsitem.cpp 
                     searchwidgets.cpp 
                     syncjob.cpp 
                     tagcreatedlg.cpp 
                     tagfilterview.cpp 
                     tagfolderview.cpp 
                     tagspopupmenu.cpp 
                     welcomepageview.cpp 
    )

KDE4_ADD_UI_FILES(digikam_bin_SRCS ../libs/dialogs/deletedialogbase.ui)

QT4_ADD_DBUS_ADAPTOR(digikam_bin_SRCS org.kde.digikam.xml digikamapp.h Digikam::DigikamApp)

KDE4_ADD_EXECUTABLE(digikam_bin ${digikam_bin_SRCS})

TARGET_LINK_LIBRARIES(digikam_bin
                      ${KDE4_KPARTS_LIBS}
                      ${KDE4_KABC_LIBS}
                      ${GPHOTO2_LIBRARIES}
                      digikam
                      digikamdatabase
                     )

SET_TARGET_PROPERTIES(digikam_bin PROPERTIES OUTPUT_NAME digikam)

INSTALL(TARGETS digikam_bin DESTINATION ${BIN_INSTALL_DIR})


# Others Files to install #######################################################

INSTALL(FILES digikam.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
INSTALL(FILES digikamui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikam_export.h DESTINATION ${INCLUDE_INSTALL_DIR})
