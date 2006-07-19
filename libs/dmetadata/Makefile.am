SUBDIRS = loaders 
METASOURCES = AUTO
KDE_CXXFLAGS = $(USE_EXCEPTIONS)

noinst_LTLIBRARIES = libdmetadata.la

libdmetadata_la_SOURCES = dmetadata.cpp

libdmetadata_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

libdmetadata_la_LIBADD  = $(top_builddir)/digikam/libs/dmetadata/loaders/libdmetadataloader.la
	                 
INCLUDES = -I$(top_srcdir)/digikam/libs/dmetadata/loaders \
	   -I$(top_srcdir)/digikam/libs/dimg \
	   -I$(top_srcdir)/digikam/libs/dcraw \
	   -I$(top_srcdir)/digikam/digikam \
	   -I$(top_srcdir)/digikam/ \
	   $(all_includes)

digikaminclude_HEADERS = dmetadata.h photoinfocontainer.h 
digikamincludedir = $(includedir)/digikam
