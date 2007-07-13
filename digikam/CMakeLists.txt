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

                     # Just to debug all image plugins with showfoto. 
                     # To remove later.
                     #${libdialogshowfoto_SRCS}
                     #${libshowfoto_SRCS}

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

#                      kdateedit.cpp 
#                      metadatahub.cpp 
#                      digikamapp.cpp 
                      album.cpp 
#                      albumiconview.cpp 
                      albumicongroupitem.cpp 
                      albumiconitem.cpp 
                      albumitemhandler.cpp 
#                      albumfiletip.cpp 
#                      albumfolderview.cpp 
                      albumhistory.cpp 
                      albumlister.cpp 
#                      albummanager.cpp 
                      albumpropsedit.cpp 
                      albumsettings.cpp 
#                      albumthumbnailloader.cpp 
                      albumwidgetstack.cpp 
                      cameradragobject.cpp 
                      cameralist.cpp 
                      cameratype.cpp 
                      datefolderview.cpp 
#                      dcopiface.cpp 
                      digikamfirstrun.cpp 
#                      digikamview.cpp 
                      dio.cpp 
                      dragobjects.cpp 
                      firstrun.cpp 
#                      folderitem.cpp 
                      folderview.cpp 
#                      iconview.cpp 
                      icongroupitem.cpp 
                      iconitem.cpp 
                      imageattributeswatch.cpp 
                      imagepreviewview.cpp 
                      kdatetimeedit.cpp 
#                      kdatepickerpopup.cpp 
                      kipiinterface.cpp 
#                      mediaplayerview.cpp 
#                      monthwidget.cpp 
                      pixmapmanager.cpp 
                      ratingpopupmenu.cpp 
                      ratingwidget.cpp 
#                      scanlib.cpp 
#                      searchadvanceddialog.cpp 
#                      searchfolderview.cpp 
                      searchquickdialog.cpp 
#                      searchresultsview.cpp 
#                      searchresultsitem.cpp 
#                      searchwidgets.cpp 
#                      syncjob.cpp 
#                      tagcreatedlg.cpp 
#                      tagfilterview.cpp 
#                      tagfolderview.cpp 
#                      tagspopupmenu.cpp 
                      welcomepageview.cpp 
                      main.cpp
   )

KDE4_ADD_UI_FILES(digikam_LIB_SRCS ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs/deletedialogbase.ui)


#kde4_add_dcop_skels(libdigikam_LIB_SRCS MESSAGE(STATUS "DCOP stuff was remove into kde4, convert it to dbus please. kde4_add_dcop_skels macro doesn't exist now. We keep it just to remember to port to dbus")dcopiface.h )

KDE4_ADD_LIBRARY(digikam SHARED ${digikam_LIB_SRCS})

TARGET_LINK_LIBRARIES(digikam
                      ${KDE4_KDECORE_LIBS} 
                      ${KDE4_KIO_LIBS} 
                      ${KDE4_KHTML_LIBS} 
                      ${KDE4_KDEUI_LIBS} 
                      ${KDE4_KUTILS_LIBS} 
                      ${SQLITE_LIBRARIES}
                      ${GPHOTO2_LIBRARIES}
                      ${JPEG_LIBRARIES}
                      ${TIFF_LIBRARIES}
                      ${PNG_LIBRARIES}
                      ${LCMS_LIBRARIES}
                      ${JASPER_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
                      ${LCMS_LIBRARIES}
                      ${KEXIV2_LIBRARIES}
                      ${KIPI_LIBRARIES}
	      )

SET_TARGET_PROPERTIES(digikam PROPERTIES VERSION 1.0.0 SOVERSION 1 )
INSTALL(TARGETS digikam DESTINATION ${LIB_INSTALL_DIR} )


# digiKam binary progam #########################################################

#SET(digikam_bin_SRCS main.cpp )

#KDE4_AUTOMOC(${digikam_bin_SRCS})

#KDE4_ADD_EXECUTABLE(digikam_bin ${digikam_bin_SRCS})

#TARGET_LINK_LIBRARIES(digikam_bin 
#                       ${KDE4_KPARTS_LIBS} 
#                      )

#SET_TARGET_PROPERTIES(digikam_bin PROPERTIES OUTPUT_NAME digikam)

#INSTALL(TARGETS digikam_bin DESTINATION ${BIN_INSTALL_DIR})


# Others Files to install #######################################################

INSTALL( FILES digikam.desktop DESTINATION ${XDG_APPS_DIR} )
INSTALL( FILES digikamui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam )
INSTALL( FILES digikam_export.h DESTINATION ${INCLUDE_INSTALL_DIR} )


#original Makefile.am contents follow:

#METASOURCES = AUTO  
#
#INCLUDES = -I$(top_srcdir)/digikam/libs/histogram \
#	   -I$(top_srcdir)/digikam/libs/levels \
#	   -I$(top_srcdir)/digikam/libs/curves \
#	   -I$(top_srcdir)/digikam/libs/themeengine \
#	   -I$(top_srcdir)/digikam/libs/widgets/common \
#	   -I$(top_srcdir)/digikam/libs/thumbbar \
#	   -I$(top_srcdir)/digikam/libs/jpegutils \
#	   -I$(top_srcdir)/digikam/libs/dcraw \
#	   -I$(top_srcdir)/digikam/libs/dimg \
#	   -I$(top_srcdir)/digikam/libs/dmetadata \
#	   -I$(top_srcdir)/digikam/libs/imageproperties \
#	   -I$(top_srcdir)/digikam/libs/threadimageio \
#	   -I$(top_srcdir)/digikam/libs/database \
#	   -I$(top_builddir)/digikam/libs/dialogs \
#	   -I$(top_srcdir)/digikam/libs/dialogs \
#	   -I$(top_srcdir)/digikam/utilities/cameragui \
#	   -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
#	   -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
#	   -I$(top_srcdir)/digikam/utilities/splashscreen \
#	   -I$(top_srcdir)/digikam/utilities/setup \
#	   -I$(top_srcdir)/digikam/utilities/slideshow \
#	   -I$(top_srcdir)/digikam/utilities/batch \
#	   -I$(top_srcdir)/digikam/utilities/lighttable \
#	   $(LIBKEXIV2_CFLAGS) \
#	   $(LIBKIPI_CFLAGS) $(LIBKDCRAW_CFLAGS) $(GPHOTO_CFLAGS) $(all_includes) 
#
#
## -- shared digiKam library rules -----------------------------------------------
#       
#lib_LTLIBRARIES = libdigikam.la
#
## NOTE from Gilles (30-11-06): kdateedit.cpp and metadatahub.cpp must be placed on the top 
## of source file list to unbreak compilation with './configure -enable-final' option. 
## I suspect a problem with X11 header included into albumfolderview.cpp witch redefine 'enum' type.
#libdigikam_la_SOURCES = kdateedit.cpp \
#	                metadatahub.cpp \
#	                digikamapp.cpp \
#	                album.cpp \
#	                albumiconview.cpp \
#	                albumicongroupitem.cpp \
#	                albumiconitem.cpp \
#	                albumitemhandler.cpp \
#	                albumfiletip.cpp \
#	                albumfolderview.cpp \
#	                albumhistory.cpp \
#	                albumlister.cpp \
#	                albummanager.cpp \
#	                albumpropsedit.cpp \
#	                albumsettings.cpp \
#	                albumthumbnailloader.cpp \
#	                albumwidgetstack.cpp \
#	                cameradragobject.cpp \
#	                cameralist.cpp \
#	                cameratype.cpp \
#	                datefolderview.cpp \
#	                dcopiface.cpp \
#	                dcopiface.skel \
#	                digikamfirstrun.cpp \
#	                digikamview.cpp \
#		dio.cpp \
#	                dragobjects.cpp \
#	                firstrun.cpp \
#	                folderitem.cpp \
#	                folderview.cpp \
#	                iconview.cpp \
#	                icongroupitem.cpp \
#	                iconitem.cpp \
#	                imageattributeswatch.cpp \
#	                imagepreviewview.cpp \
#	                kdatetimeedit.cpp \
#	                kdatepickerpopup.cpp \
#	                kipiinterface.cpp \
#	                mediaplayerview.cpp \
#	                monthwidget.cpp \
#	                pixmapmanager.cpp \
#	                ratingpopupmenu.cpp \
#	                ratingwidget.cpp \
#	                scanlib.cpp \
#	                searchadvanceddialog.cpp \
#	                searchfolderview.cpp \
#	                searchquickdialog.cpp \
#	                searchresultsview.cpp \
#	                searchresultsitem.cpp \
#	                searchwidgets.cpp \
#	                syncjob.cpp \
#	                tagcreatedlg.cpp \
#	                tagfilterview.cpp \
#	                tagfolderview.cpp \
#	                tagspopupmenu.cpp \
#	                welcomepageview.cpp
#
#libdigikam_la_LIBADD = $(LIB_KIO) $(LIB_SQLITE3) $(LIB_KABC) $(LIB_KHTML) \
#	               $(LIBKIPI_LIBS) $(LIB_KUTILS) \
#	               $(top_builddir)/digikam/sqlite/libsqlite.la \
#	               $(top_builddir)/digikam/libs/thumbbar/libthumbbar.la \
#	               $(top_builddir)/digikam/libs/themeengine/libthemeengine.la \
#	               $(top_builddir)/digikam/libs/widgets/libwidgets.la \
#	               $(top_builddir)/digikam/libs/dialogs/libdialog.la \
#	               $(top_builddir)/digikam/libs/jpegutils/libjpegutils.la \
#	               $(top_builddir)/digikam/libs/dimg/libdimg.la \
#	               $(top_builddir)/digikam/libs/imageproperties/libimagepropertiesdigikam.la \
#	               $(top_builddir)/digikam/libs/threadimageio/libthreadimageio.la \
#	               $(top_builddir)/digikam/libs/greycstoration/libgreycstoration.la \
#	               $(top_builddir)/digikam/libs/database/libdatabase.la \
#	               $(top_builddir)/digikam/utilities/batch/libbatch.la \
#	               $(top_builddir)/digikam/utilities/slideshow/libslideshow.la \
#	               $(top_builddir)/digikam/utilities/cameragui/libcameragui.la \
#	               $(top_builddir)/digikam/utilities/imageeditor/canvas/libdimgcanvas.la \
#	               $(top_builddir)/digikam/utilities/imageeditor/editor/libdimgeditor.la \
#	               $(top_builddir)/digikam/utilities/splashscreen/libsplashscreen.la \
#	               $(top_builddir)/digikam/utilities/setup/libsetup.la \
#	               $(top_builddir)/digikam/utilities/lighttable/liblighttable.la
#
#libdigikam_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined
#
## -- main digiKam binary target file rules ----------------------------------------------
#
#bin_PROGRAMS = digikam
#
#digikam_SOURCES = main.cpp
#digikam_LDADD   = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) \
#		          $(LIBSOCKET) $(LIB_KFILE) $(LIB_KPARTS) $(LIB_KUTILS) \
#		          libdigikam.la
#
#digikam_LDFLAGS = $(KDE_RPATH) $(all_libraries) $(LIB_KUTILS) 
#
#rcdir = $(kde_datadir)/digikam
#rc_DATA = digikamui.rc
#
#dummy_digikam.cpp:
#	echo > dummy_digikam.cpp
#
#xdg_apps_DATA = digikam.desktop
#
#include_HEADERS = digikam_export.h
#
#include ../../admin/Doxyfile.am
#noinst_HEADERS = dcopiface.h
