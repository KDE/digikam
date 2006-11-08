SUBDIRS = loaders filters 
METASOURCES = AUTO

noinst_LTLIBRARIES = libdimg.la

libdimg_la_SOURCES = dimg.cpp dimgscale.cpp dcolor.cpp dcolorcomposer.cpp ddebug.cpp

libdimg_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LCMS_LIBS) 

libdimg_la_LIBADD  = $(top_builddir)/digikam/libs/dcraw/libdcraw.la \
	                 $(top_builddir)/digikam/libs/histogram/libhistogram.la \
	                 $(top_builddir)/digikam/libs/levels/liblevels.la \
	                 $(top_builddir)/digikam/libs/curves/libcurves.la \
	                 $(top_builddir)/digikam/libs/dimg/loaders/libdimgloaders.la \
	                 $(top_builddir)/digikam/libs/dimg/filters/libdimgfilters.la \
	                 $(top_builddir)/digikam/libs/dmetadata/libdmetadata.la

INCLUDES = $(all_includes) -I$(top_srcdir)/digikam/libs/dimg/loaders \
	                       -I$(top_srcdir)/digikam/libs/dimg/filters \
	                       -I$(top_srcdir)/digikam/libs/dmetadata \
	                       -I$(top_srcdir)/digikam/libs/dcraw \
	                       -I$(top_srcdir)/digikam/digikam

digikaminclude_HEADERS = dimg.h dcolor.h dcolorpixelaccess.h dcolorcomposer.h \
                         dcolorblend.h rawdecodingsettings.h ddebug.h
digikamincludedir = $(includedir)/digikam
