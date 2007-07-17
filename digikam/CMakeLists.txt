#Require by Greycstoration algorithm (CImg.h)
ADD_DEFINITIONS(${KDE4_ENABLE_EXCEPTIONS})

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/histogram 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/levels 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/lprof 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/curves 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/whitebalance 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/greycstoration 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/themeengine 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/metadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/thumbbar 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/jpegutils 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/loaders 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/imageproperties 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/threadimageio 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/database
                     ${CMAKE_SOURCE_DIR}/digikam/libs/database/sqlite2
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/cameragui 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/tools 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/setup 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/slideshow 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/batch 
                     ${CMAKE_SOURCE_DIR}/digikam/utilities/lighttable  
                   )


# Shared libdigikam ################################################################


SET(digikam_LIB_SRCS 
                     ${libcameragui_SRCS}
                     ${liblighttable_SRCS}
                     ${libimagepropertiesdigikam_SRCS}
                     ${libdatabase_SRCS}
                     ${libsqlite2_SRCS}
                     ${libsetup_SRCS}
                     ${libbatch_SRCS}
                     ${libdialog_SRCS}
                     ${libdimgeditor_SRCS}

                     ${libslideshow_SRCS}
                     ${libdimgcanvas_SRCS}
                     ${libdimgeditortools_SRCS}
                     ${libthumbbar_SRCS}
                     ${libthemeengine_SRCS}
                     ${libcommonwidgets_SRCS}
                     ${libiccprofileswidgets_SRCS}
                     ${libmetadatawidgets_SRCS}
                     ${libimagepluginswidgets_SRCS}
                     ${libjpegutils_SRCS}
                     ${libdimg_SRCS}
                     ${libdimgloaders_SRCS}
                     ${libdimgfilters_SRCS}
                     ${liblprof_SRCS}
                     ${libthreadimageio_SRCS}
                     ${libhistogram_SRCS}
                     ${liblevels_SRCS}
                     ${libcurves_SRCS}
                     ${libwhitebalance_SRCS}
                     ${libgreycstoration_SRCS}
                     ${libdmetadata_SRCS}

                      kdateedit.cpp 
                      metadatahub.cpp 
                      digikamapp.cpp 
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
                      albumsettings.cpp 
                      albumthumbnailloader.cpp 
                      albumwidgetstack.cpp 
                      cameradragobject.cpp 
                      cameralist.cpp 
                      cameratype.cpp 
                      datefolderview.cpp 
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
                      kdatetimeedit.cpp 
                      kdatepickerpopup.cpp 
                      kipiinterface.cpp 
                      mediaplayerview.cpp 
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
                      main.cpp
   )

KDE4_ADD_UI_FILES(digikam_LIB_SRCS ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs/deletedialogbase.ui)

qt4_add_dbus_adaptor(digikam_LIB_SRCS org.kde.digikam.xml digikamapp.h Digikam::DigikamApp)


KDE4_ADD_LIBRARY(digikam SHARED ${digikam_LIB_SRCS})

TARGET_LINK_LIBRARIES(digikam
                      ${KDE4_KDECORE_LIBS} 
                      ${KDE4_KIO_LIBS} 
                      ${KDE4_KHTML_LIBS} 
                      ${KDE4_KDEUI_LIBS} 
                      ${KDE4_KUTILS_LIBS} 
                      ${KDE4_KABC_LIBS}

                      ${SQLITE_LIBRARIES}
                      ${GPHOTO2_LIBRARIES}
                      ${JPEG_LIBRARIES}
                      ${TIFF_LIBRARIES}
                      ${PNG_LIBRARIES}
                      ${LCMS_LIBRARIES}
                      ${JASPER_LIBRARIES}
                      ${LCMS_LIBRARIES}

                      ${KDCRAW_LIBRARIES}
                      ${KEXIV2_LIBRARIES}
                      ${KIPI_LIBRARIES}
	      )

SET_TARGET_PROPERTIES(digikam PROPERTIES VERSION 1.0.0 SOVERSION 1 )
INSTALL(TARGETS digikam DESTINATION ${LIB_INSTALL_DIR} )

# digiKam binary progam #########################################################   
      
SET(digikam_bin_SRCS main.cpp)   
      
KDE4_ADD_EXECUTABLE(digikam_bin ${digikam_bin_SRCS})   
      
TARGET_LINK_LIBRARIES(digikam_bin ${KDE4_KPARTS_LIBS} digikam)   
      
SET_TARGET_PROPERTIES(digikam_bin PROPERTIES OUTPUT_NAME digikam)   
      
INSTALL(TARGETS digikam_bin DESTINATION ${BIN_INSTALL_DIR})   
      
      
# Others Files to install #######################################################   
      
INSTALL( FILES digikam.desktop DESTINATION ${XDG_APPS_DIR} )   
INSTALL( FILES digikamui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam )   
INSTALL( FILES digikam_export.h DESTINATION ${INCLUDE_INSTALL_DIR} )
