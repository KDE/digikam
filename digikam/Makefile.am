SUBDIRS = pics

lib_LTLIBRARIES = libdigikam.la
bin_PROGRAMS = digikam

INCLUDES = \
	-I$(top_srcdir)/digikam/sqlite \
	-I$(top_srcdir)/digikam/libs/histogram \
	-I$(top_srcdir)/digikam/libs/filters \
	-I$(top_srcdir)/digikam/libs/levels \
	-I$(top_srcdir)/digikam/libs/curves \
	-I$(top_srcdir)/digikam/libs/themeengine \
	-I$(top_srcdir)/digikam/libs/widgets \
	-I$(top_srcdir)/digikam/libs/thumbbar \
	-I$(top_srcdir)/digikam/utilities/cameragui \
	-I$(top_srcdir)/digikam/utilities/imageeditor \
	-I$(top_srcdir)/digikam/utilities/splashscreen \
	-I$(top_srcdir)/digikam/utilities/setup \
	-I$(top_srcdir)/digikam/ \
	$(LIBKIPI_CFLAGS) $(LIBKEXIF_CFLAGS) $(all_includes)

libdigikam_la_SOURCES = albummanager.cpp album.cpp \
			albumitemhandler.cpp albumdb.cpp \
			albumlister.cpp albumfilecopymove.cpp \
			thumbnailjob.cpp dragobjects.cpp \
			listview.cpp listitem.cpp \
			thumbview.cpp thumbitem.cpp \
			digikamapp.cpp digikamview.cpp \
		        albumfolderview.cpp albumfolderitem.cpp \
		        albumiconview.cpp albumiconitem.cpp \
		        albumpropsedit.cpp imagedescedit.cpp \
		        albumsettings.cpp imageproperties.cpp \
			imagepropertiesgeneral.cpp imagepropertiesexif.cpp \
			imagepropertieshistogram.cpp \
		        cameralist.cpp cameratype.cpp cameradragobject.cpp \
		        digikamfirstrun.cpp firstrunUI.ui \
			kipiinterface.cpp tagcreatedlg.cpp \
			thumbdb.cpp albumfiletip.cpp digikamio.cpp \
			busyprogressbar.cpp tagspopupmenu.cpp \
			syncjob.cpp albumhistory.cpp

libdigikam_la_LIBADD  = $(LIB_KIO) $(LIB_GDBM) \
			$(LIBKEXIF_LIBS) $(LIBKIPI_LIBS) \
		        $(top_builddir)/digikam/sqlite/libsqlite.la \
		        $(top_builddir)/digikam/libs/histogram/libhistogram.la \
		        $(top_builddir)/digikam/libs/levels/liblevels.la \
		        $(top_builddir)/digikam/libs/curves/libcurves.la \
		        $(top_builddir)/digikam/libs/filters/libfilters.la \
		        $(top_builddir)/digikam/libs/thumbbar/libthumbbar.la \
		        $(top_builddir)/digikam/libs/themeengine/libthemeengine.la \
		        $(top_builddir)/digikam/libs/widgets/libwidgets.la \
		        $(top_builddir)/digikam/utilities/cameragui/libcameragui.la \
		        $(top_builddir)/digikam/utilities/imageeditor/libimagecanvas.la \
		        $(top_builddir)/digikam/utilities/imageeditor/libimageeditor.la \
		        $(top_builddir)/digikam/utilities/splashscreen/libsplashscreen.la \
		        $(top_builddir)/digikam/utilities/setup/libsetup.la

libdigikam_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) \
			-no-undefined


digikam_SOURCES = main.cpp
digikam_LDADD   = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) \
		  $(LIBSOCKET) $(LIB_KFILE) $(LIB_KPARTS)  \
		  libdigikam.la

digikam_LDFLAGS = $(KDE_RPATH) $(all_libraries) $(LIB_IMLIB2)

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

