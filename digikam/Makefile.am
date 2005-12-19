SUBDIRS = pics

lib_LTLIBRARIES = libdigikam.la
bin_PROGRAMS = digikam

INCLUDES = -I$(top_srcdir)/digikam/sqlite \
	   -I$(top_srcdir)/digikam/libs/histogram \
	   -I$(top_srcdir)/digikam/libs/filters \
	   -I$(top_srcdir)/digikam/libs/levels \
	   -I$(top_srcdir)/digikam/libs/curves \
	   -I$(top_srcdir)/digikam/libs/themeengine \
	   -I$(top_srcdir)/digikam/libs/widgets \
	   -I$(top_srcdir)/digikam/libs/thumbbar \
	   -I$(top_srcdir)/digikam/libs/jpegutils \
	   -I$(top_srcdir)/digikam/libs/dcraw \
	   -I$(top_srcdir)/digikam/libs/dimg \
	   -I$(top_srcdir)/digikam/libs/imageproperties \
	   -I$(top_srcdir)/digikam/utilities/cameragui \
	   -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
	   -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
	   -I$(top_srcdir)/digikam/utilities/splashscreen \
	   -I$(top_srcdir)/digikam/utilities/setup \
	   -I$(top_srcdir)/digikam/ \
	   $(LIBKIPI_CFLAGS) $(LIBKEXIF_CFLAGS) $(all_includes)

libdigikam_la_SOURCES = firstrunUI.ui albummanager.cpp album.cpp \
			albumitemhandler.cpp albumdb.cpp albumdb_sqlite2.cpp \
			albumlister.cpp dragobjects.cpp \
			iconview.cpp icongroupitem.cpp iconitem.cpp \
			digikamapp.cpp digikamview.cpp \
		    albumfolderview.cpp \
		    albumiconview.cpp albumicongroupitem.cpp albumiconitem.cpp \
		    albumpropsedit.cpp albumsettings.cpp \
		    cameralist.cpp cameratype.cpp cameradragobject.cpp \
			kipiinterface.cpp tagcreatedlg.cpp \
			albumfiletip.cpp tagspopupmenu.cpp \
			syncjob.cpp albumhistory.cpp scanlib.cpp \
			imageinfo.cpp \
			datefolderview.cpp pixmapmanager.cpp \
			tagfolderview.cpp tagfilterview.cpp monthwidget.cpp \
			searchfolderview.cpp dio.cpp \
			searchresultsview.cpp searchresultsitem.cpp \
			searchquickdialog.cpp searchadvanceddialog.cpp \
			searchwidgets.cpp kdateedit.cpp kdatetimeedit.cpp \
			kdatepickerpopup.cpp folderview.cpp folderitem.cpp \
			upgradedb_sqlite2tosqlite3.cpp squeezedcombobox.cpp \
			ratingwidget.cpp digikamfirstrun.cpp \
			dcopiface.cpp dcopiface.skel




libdigikam_la_LIBADD  = $(LIB_KIO) $(LIB_SQLITE3) $(LIB_KABC) \
			$(LIBKEXIF_LIBS) $(LIBKIPI_LIBS) $(LIB_KUTILS) \
		        $(top_builddir)/digikam/sqlite/libsqlite.la \
		        $(top_builddir)/digikam/libs/filters/libfilters.la \
		        $(top_builddir)/digikam/libs/thumbbar/libthumbbar.la \
		        $(top_builddir)/digikam/libs/themeengine/libthemeengine.la \
		        $(top_builddir)/digikam/libs/widgets/libwidgets.la \
		        $(top_builddir)/digikam/libs/dialogs/libdialog.la \
		        $(top_builddir)/digikam/libs/jpegutils/libjpegutils.la \
		        $(top_builddir)/digikam/libs/imageproperties/libimagepropertiesdigikam.la \
	            $(top_builddir)/digikam/libs/threadimageio/libthreadimageio.la \
		        $(top_builddir)/digikam/utilities/cameragui/libcameragui.la \
		        $(top_builddir)/digikam/utilities/imageeditor/canvas/libdimgcanvas.la \
		        $(top_builddir)/digikam/utilities/imageeditor/editor/libdimgeditor.la \
		        $(top_builddir)/digikam/utilities/splashscreen/libsplashscreen.la \
		        $(top_builddir)/digikam/utilities/setup/libsetup.la

libdigikam_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined


digikam_SOURCES = main.cpp
digikam_LDADD   = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) \
		  $(LIBSOCKET) $(LIB_KFILE) $(LIB_KPARTS) $(LIB_KUTILS) \
		  libdigikam.la

digikam_LDFLAGS = $(KDE_RPATH) $(all_libraries) $(LIB_KUTILS)

dummy_digikam.cpp:
	echo > dummy_digikam.cpp

METASOURCES = AUTO  

# this is where the kdelnk file will go 
kdelnkdir   = $(kde_appsdir)/Graphics
kdelnk_DATA = digikam.desktop


# Uncomment the following two lines if you add a ui.rc file for your application to make use of
# KDE2 XML GUI builing
rcdir = $(kde_datadir)/digikam
rc_DATA = digikamui.rc

include_HEADERS = digikam_export.h

KDE_OPTIONS = nofinal

include ../../admin/Doxyfile.am
noinst_HEADERS = dcopiface.h
