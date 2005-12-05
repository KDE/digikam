METASOURCES = AUTO

noinst_LTLIBRARIES = libdimgcanvas.la

libdimgcanvas_la_SOURCES = dimginterface.cpp \
	                       canvas.cpp undocache.cpp \
			               undoaction.cpp undomanager.cpp \
			               imagepluginloader.cpp imageplugin.cpp

libdimgcanvas_la_LIBADD  = $(top_builddir)/digikam/libs/dimg/libdimg.la 

libdimgcanvas_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) \
	                   $(LIBKEXIF_LIBS) $(LIB_TIFF) 

INCLUDES= -I$(top_srcdir)/digikam/digikam \
	  -I$(top_srcdir)/digikam/libs/dimg \
	  -I$(top_srcdir)/digikam/libs/dimg/filters \
	  -I$(top_srcdir)/digikam/libs/histogram \
	  -I$(top_srcdir)/digikam/utilities/splashscreen \
	  $(LIBKEXIF_CFLAGS) $(all_includes) 

digikaminclude_HEADERS = imageplugin.h 

digikamincludedir = $(includedir)/digikam

KDE_OPTIONS = nofinal
