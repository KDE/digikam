SUBDIRS = loaders
METASOURCES = AUTO

noinst_LTLIBRARIES = libdimg.la

libdimg_la_SOURCES = dimg.cpp dimgloader.cpp dimgscale.cpp colormodifier.cpp \
	                 icctransform.cpp 

libdimg_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LCMS_LIBS) \
	                 $(top_builddir)/digikam/libs/dcraw/libdcraw.la \
	                 $(top_builddir)/digikam/libs/dimg/loaders/libdimgloaders.la 

INCLUDES = $(all_includes) -I$(top_srcdir)/digikam/libs/dimg/loaders 

digikaminclude_HEADERS = dimg.h dimgloader.h colormodifier.h icctransform.h 
digikamincludedir = $(includedir)/digikam
