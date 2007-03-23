METASOURCES = AUTO

noinst_LTLIBRARIES = libdimgcanvas.la

libdimgcanvas_la_SOURCES = dimginterface.cpp colorcorrectiondlg.cpp \
	                       canvas.cpp undocache.cpp \
	                       undoaction.cpp undomanager.cpp \
	                       imagepluginloader.cpp imageplugin.cpp

libdimgcanvas_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TIFF) 

INCLUDES = -I$(top_srcdir)/digikam/digikam \
	       -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/libs/dimg/filters \
	       -I$(top_srcdir)/digikam/libs/dmetadata \
	       -I$(top_srcdir)/digikam/libs/dialogs \
	       -I$(top_srcdir)/digikam/libs/histogram \
	       -I$(top_srcdir)/digikam/libs/threadimageio \
	       -I$(top_srcdir)/digikam/utilities/splashscreen \
	       -I$(top_srcdir)/digikam/libs/widgets/imageplugins \
	       -I$(top_srcdir)/digikam/libs/widgets/common \
	       $(LIBKEXIV2_CFLAGS) $(LIBKDCRAW_CFLAGS) $(all_includes) 

digikaminclude_HEADERS = imageplugin.h 

digikamincludedir = $(includedir)/digikam

