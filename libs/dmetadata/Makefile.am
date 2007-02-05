METASOURCES = AUTO

noinst_LTLIBRARIES = libdmetadata.la

libdmetadata_la_SOURCES = dmetadata.cpp

libdmetadata_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

libdmetadata_la_LIBADD  = $(LIBKEXIV2_LIBS)  
	                 
INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
	   -I$(top_srcdir)/digikam/libs/dcraw \
	   -I$(top_srcdir)/digikam/digikam \
	   $(all_includes)

digikaminclude_HEADERS = dmetadata.h photoinfocontainer.h 
digikamincludedir = $(includedir)/digikam
