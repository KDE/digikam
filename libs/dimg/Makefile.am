SUBDIRS = loaders
METASOURCES = AUTO

noinst_LTLIBRARIES = libdimg.la

libdimg_la_SOURCES = dimg.cpp dimgloader.cpp dimgscale.cpp colormodifier.cpp \
	                 icctransform.cpp dcolor.cpp 

libdimg_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LCMS_LIBS) 

libdimg_la_LIBADD  = $(top_builddir)/digikam/libs/dcraw/libdcraw.la \
	                 $(top_builddir)/digikam/libs/dimg/loaders/libdimgloaders.la 

INCLUDES = $(all_includes) -I$(top_srcdir)/digikam/libs/dimg/loaders \
	                   -I$(top_srcdir)/digikam/digikam 

digikaminclude_HEADERS = dimg.h dimgloader.h colormodifier.h icctransform.h dcolor.h 
digikamincludedir = $(includedir)/digikam
