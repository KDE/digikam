METASOURCES = AUTO  

INCLUDES = -I$(top_srcdir)/digikam/sqlite \
	   -I$(top_srcdir)/digikam/libs/histogram \
	   -I$(top_srcdir)/digikam/libs/levels \
	   -I$(top_srcdir)/digikam/libs/curves \
	   -I$(top_srcdir)/digikam/libs/themeengine \
	   -I$(top_srcdir)/digikam/libs/widgets/common \
	   -I$(top_srcdir)/digikam/libs/thumbbar \
	   -I$(top_srcdir)/digikam/libs/jpegutils \
	   -I$(top_srcdir)/digikam/libs/dcraw \
	   -I$(top_srcdir)/digikam/libs/dimg \
	   -I$(top_srcdir)/digikam/libs/dmetadata \
	   -I$(top_srcdir)/digikam/libs/imageproperties \
	   -I$(top_srcdir)/digikam/libs/threadimageio \
	   -I$(top_builddir)/digikam/libs/dialogs \
	   -I$(top_srcdir)/digikam/libs/dialogs \
	   -I$(top_srcdir)/digikam/utilities/cameragui \
	   -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
	   -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
	   -I$(top_srcdir)/digikam/utilities/splashscreen \
	   -I$(top_srcdir)/digikam/utilities/setup \
	   -I$(top_srcdir)/digikam/utilities/slideshow \
	   -I$(top_srcdir)/digikam/utilities/batch \
	   $(LIBKEXIV2_CFLAGS) \
	   $(LIBKIPI_CFLAGS) $(LIBKDCRAW_CFLAGS) $(GPHOTO_CFLAGS) $(all_includes) 


# -- shared digiKam library rules -----------------------------------------------
       
lib_LTLIBRARIES = libdigikam.la

# NOTE from Gilles (30-11-06): kdateedit.cpp and metadatahub.cpp must be placed on the top 
# of source file list to unbreak compilation with './configure -enable-final' option. 
# I suspect a problem with X11 header included into albumfolderview.cpp witch redefine 'enum' type.
libdigikam_la_SOURCES = kdateedit.cpp \
	                metadatahub.cpp \
	                digikamapp.cpp \
	                album.cpp \
	                albumdb.cpp \
	                albumdb_sqlite2.cpp \
	                albumiconview.cpp \
	                albumicongroupitem.cpp \
	                albumiconitem.cpp \
	                albumitemhandler.cpp \
	                albumfiletip.cpp \
	                albumfolderview.cpp \
	                albumhistory.cpp \
	                albumlister.cpp \
	                albummanager.cpp \
	                albumpropsedit.cpp \
	                albumsettings.cpp \
	                albumthumbnailloader.cpp \
	                albumwidgetstack.cpp \
	                cameradragobject.cpp \
	                cameralist.cpp \
	                cameratype.cpp \
	                datefolderview.cpp \
	                dcopiface.cpp \
	                dcopiface.skel \
	                digikamfirstrun.cpp \
	                digikamview.cpp \
	                dio.cpp \
	                dragobjects.cpp \
	                fastscale.cpp \
	                firstrun.cpp \
	                folderitem.cpp \
	                folderview.cpp \
	                iconview.cpp \
	                icongroupitem.cpp \
	                iconitem.cpp \
	                imageattributeswatch.cpp \
	                imageinfo.cpp \
	                imagepreviewwidget.cpp \
	                imagepreviewview.cpp \
	                kdatetimeedit.cpp \
	                kdatepickerpopup.cpp \
	                kipiinterface.cpp \
	                mediaplayerview.cpp \
	                monthwidget.cpp \
	                pixmapmanager.cpp \
	                ratingpopupmenu.cpp \
	                ratingwidget.cpp \
	                scanlib.cpp \
	                searchadvanceddialog.cpp \
	                searchfolderview.cpp \
	                searchquickdialog.cpp \
	                searchresultsview.cpp \
	                searchresultsitem.cpp \
	                searchwidgets.cpp \
	                syncjob.cpp \
	                tagcreatedlg.cpp \
	                tagfilterview.cpp \
	                tagfolderview.cpp \
	                tagspopupmenu.cpp \
	                upgradedb_sqlite2tosqlite3.cpp \
	                welcomepageview.cpp 

libdigikam_la_LIBADD = $(LIB_KIO) $(LIB_SQLITE3) $(LIB_KABC) $(LIB_KHTML) \
	               $(LIBKIPI_LIBS) $(LIB_KUTILS) $(EXIV2_LIBS) \
	               $(top_builddir)/digikam/sqlite/libsqlite.la \
	               $(top_builddir)/digikam/libs/thumbbar/libthumbbar.la \
	               $(top_builddir)/digikam/libs/themeengine/libthemeengine.la \
	               $(top_builddir)/digikam/libs/widgets/libwidgets.la \
	               $(top_builddir)/digikam/libs/dialogs/libdialog.la \
	               $(top_builddir)/digikam/libs/jpegutils/libjpegutils.la \
	               $(top_builddir)/digikam/libs/dimg/libdimg.la \
	               $(top_builddir)/digikam/libs/imageproperties/libimagepropertiesdigikam.la \
	               $(top_builddir)/digikam/libs/threadimageio/libthreadimageio.la \
	               $(top_builddir)/digikam/libs/greycstoration/libgreycstoration.la \
	               $(top_builddir)/digikam/utilities/batch/libbatch.la \
	               $(top_builddir)/digikam/utilities/slideshow/libslideshow.la \
	               $(top_builddir)/digikam/utilities/cameragui/libcameragui.la \
	               $(top_builddir)/digikam/utilities/imageeditor/canvas/libdimgcanvas.la \
	               $(top_builddir)/digikam/utilities/imageeditor/editor/libdimgeditor.la \
	               $(top_builddir)/digikam/utilities/splashscreen/libsplashscreen.la \
	               $(top_builddir)/digikam/utilities/setup/libsetup.la

libdigikam_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined

# -- main digiKam binary target file rules ----------------------------------------------

bin_PROGRAMS = digikam

digikam_SOURCES = main.cpp
digikam_LDADD   = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) \
		  $(LIBSOCKET) $(LIB_KFILE) $(LIB_KPARTS) $(LIB_KUTILS) \
		  libdigikam.la

digikam_LDFLAGS = $(KDE_RPATH) $(all_libraries) $(LIB_KUTILS) 

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamui.rc

dummy_digikam.cpp:
	echo > dummy_digikam.cpp

xdg_apps_DATA = digikam.desktop

include_HEADERS = digikam_export.h

include ../../admin/Doxyfile.am
noinst_HEADERS = dcopiface.h
