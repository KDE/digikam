METASOURCES = AUTO

noinst_LTLIBRARIES = libimageeditor.la libimagecanvas.la

libimagecanvas_la_SOURCES = imlibinterface.cpp canvas.cpp \
			    exifrestorer.cpp

libimagecanvas_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_IMLIB2) \
			    $(LIBKEXIF_LIBS) $(LIB_TIFF)

libimageeditor_la_SOURCES = imageiface.cpp imagewidget.cpp imageplugin.cpp \
			    imagepluginloader.cpp \
			    imagewindow.cpp \
			    imageresizedlg.cpp \
			    imageprint.cpp 

libimageeditor_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_KDEPRINT) \
			    $(LIB_IMLIB2) $(LIBKEXIF_LIBS)

INCLUDES= -I$(top_srcdir)/digikam/digikam \
	  -I$(top_srcdir)/digikam/libs/widgets \
	  -I$(top_srcdir)/digikam/utilities/splashscreen \
	  $(IMLIB2_CFLAGS) $(LIBKEXIF_CFLAGS) $(all_includes) 

digikaminclude_HEADERS = imageiface.h imagewidget.h imageplugin.h

digikamincludedir = $(includedir)/digikam

digikamimageeditoricondir = $(kde_datadir)/digikam/icons
digikamimageeditoricon_ICON = AUTO

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimagewindowui.rc

kde_servicetypes_DATA = digikamimageplugin.desktop

KDE_OPTIONS=nofinal
