METASOURCES = AUTO

noinst_LTLIBRARIES = libdimgeditor.la libshowfoto.la 

libdimgeditor_la_SOURCES = editorwindow.cpp imageiface.cpp \
	 		   imagewindow.cpp iofileprogressbar.cpp  

libdimgeditor_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_KDEPRINT) 

libdimgeditor_la_LIBADD = $(top_builddir)/digikam/utilities/imageeditor/tools/libdimgeditortools.la 

libshowfoto_la_SOURCES = editorwindow.cpp imageiface.cpp iofileprogressbar.cpp

libshowfoto_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_KDEPRINT) 

libshowfoto_la_LIBADD = $(top_builddir)/digikam/utilities/imageeditor/tools/libdimgeditortools.la 

INCLUDES= -I$(top_srcdir)/digikam/digikam \
	      -I$(top_srcdir)/digikam/libs/widgets/common \
	      -I$(top_srcdir)/digikam/libs/dialogs \
	      -I$(top_srcdir)/digikam/libs/dimg \
	      -I$(top_srcdir)/digikam/libs/themeengine \
	      -I$(top_srcdir)/digikam/libs/dmetadata \
	      -I$(top_srcdir)/digikam/libs/dimg/filters \
	      -I$(top_srcdir)/digikam/libs/imageproperties \
	      -I$(top_srcdir)/digikam/libs/threadimageio \
	      -I$(top_srcdir)/digikam/utilities/setup \
	      -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
	      -I$(top_srcdir)/digikam/utilities/imageeditor/tools \
	      -I$(top_builddir)/digikam/libs/dialogs \
	      $(all_includes) 

digikaminclude_HEADERS = imageiface.h 

digikamincludedir = $(includedir)/digikam

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimagewindowui.rc

kde_servicetypes_DATA = digikamimageplugin.desktop

KDE_OPTIONS = nofinal
