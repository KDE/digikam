METASOURCES = AUTO

noinst_LTLIBRARIES = libdimgcanvas.la

libdimgcanvas_la_SOURCES = dimginterface.cpp colorcorrectiondlg.cpp \
	                   canvas.cpp undocache.cpp \
		           undoaction.cpp undomanager.cpp \
		           imagepluginloader.cpp imageplugin.cpp

libdimgcanvas_la_LIBADD  = $(top_builddir)/digikam/libs/dimg/libdimg.la 

libdimgcanvas_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TIFF) 

INCLUDES= -I$(top_srcdir)/digikam/digikam \
	  -I$(top_srcdir)/digikam/libs/dimg \
	  -I$(top_srcdir)/digikam/libs/dimg/filters \
          -I$(top_srcdir)/digikam/libs/dmetadata \
          -I$(top_srcdir)/digikam/libs/dialogs \
	  -I$(top_srcdir)/digikam/libs/histogram \
	  -I$(top_srcdir)/digikam/libs/threadimageio \
	  -I$(top_srcdir)/digikam/utilities/splashscreen \
	  $(all_includes) 

digikaminclude_HEADERS = imageplugin.h 

digikamincludedir = $(includedir)/digikam

KDE_OPTIONS = nofinal
