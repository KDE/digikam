METASOURCES = AUTO

noinst_LTLIBRARIES = libdimgeditor.la libshowfoto.la 

libdimgeditor_la_SOURCES = imageiface.cpp imagewidget.cpp \
	 		   imagewindow.cpp imageresizedlg.cpp \
			   imageprint.cpp 

libdimgeditor_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_KDEPRINT) \
	 	           $(LIBKEXIF_LIBS)

libshowfoto_la_SOURCES = imageiface.cpp imagewidget.cpp imageprint.cpp imageresizedlg.cpp 

libshowfoto_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_KDEPRINT) 

INCLUDES= -I$(top_srcdir)/digikam/digikam \
	  -I$(top_srcdir)/digikam/libs/widgets \
	  -I$(top_srcdir)/digikam/libs/dimg \
	  -I$(top_srcdir)/digikam/libs/dimg/filters \
	  -I$(top_srcdir)/digikam/libs/imageproperties \
	  -I$(top_srcdir)/digikam/utilities/splashscreen \
	  -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
	  $(LIBKEXIF_CFLAGS) $(all_includes) 

digikaminclude_HEADERS = imageiface.h imagewidget.h 

digikamincludedir = $(includedir)/digikam

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimagewindowui.rc

kde_servicetypes_DATA = digikamimageplugin.desktop

KDE_OPTIONS = nofinal
