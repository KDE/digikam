METASOURCES = AUTO

noinst_LTLIBRARIES = libimageeditor.la

libimageeditor_la_SOURCES = imlibinterface.cpp imageiface.cpp \
			    imagewidget.cpp imageplugin.cpp \
			    imagepluginloader.cpp \
			    imagewindow.cpp imageguiclient.cpp \
			    canvas.cpp imageresizedlg.cpp \
			    imagecommentedit.cpp imageprint.cpp \
			    exifrestorer.cpp imagerotatedlg.cpp 

libimageeditor_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_KDEPRINT) $(LIB_IMLIB2) $(LIB_KEXIF) -ltiff

INCLUDES= -I$(top_srcdir)/digikam/digikam \
	  -I$(top_srcdir)/digikam/libs/guibuilder \
	  $(IMLIB2_CFLAGS) $(all_includes) 

digikaminclude_HEADERS = imageiface.h imagewidget.h imageplugin.h

digikamincludedir = $(includedir)/digikam

digikamimageeditoricondir = $(kde_datadir)/digikam/icons
digikamimageeditoricon_ICON = AUTO

kde_servicetypes_DATA = digikamimageplugin.desktop

