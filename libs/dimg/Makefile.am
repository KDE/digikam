SUBDIRS = loaders filters 
METASOURCES = AUTO

noinst_LTLIBRARIES = libdimg.la

libdimg_la_SOURCES = dimg.cpp dimgscale.cpp dcolor.cpp dcolorcomposer.cpp ddebug.cpp 

libdimg_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) 

libdimg_la_LIBADD  = $(top_builddir)/digikam/libs/histogram/libhistogram.la \
	             $(top_builddir)/digikam/libs/levels/liblevels.la \
	             $(top_builddir)/digikam/libs/curves/libcurves.la \
	             $(top_builddir)/digikam/libs/whitebalance/libwhitebalance.la \
	             $(top_builddir)/digikam/libs/dimg/loaders/libdimgloaders.la \
	             $(top_builddir)/digikam/libs/dimg/filters/libdimgfilters.la \
	             $(top_builddir)/digikam/libs/dmetadata/libdmetadata.la \
	             $(LIBKDCRAW_LIBS) $(LCMS_LIBS) 

INCLUDES = $(all_includes) $(LIBKDCRAW_CFLAGS) \
	   -I$(top_srcdir)/digikam/libs/dimg/loaders \
	   -I$(top_srcdir)/digikam/libs/dimg/filters \
	   -I$(top_srcdir)/digikam/libs/dmetadata \
	   -I$(top_srcdir)/digikam/digikam

digikaminclude_HEADERS = dimg.h dcolor.h dcolorpixelaccess.h dcolorcomposer.h \
                         dcolorblend.h ddebug.h
digikamincludedir = $(includedir)/digikam
