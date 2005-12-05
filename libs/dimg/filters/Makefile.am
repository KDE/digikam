METASOURCES = AUTO

noinst_LTLIBRARIES = libdimgfilters.la

libdimgfilters_la_SOURCES = bcgmodifier.cpp hslmodifier.cpp

libdimgfilters_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = $(all_includes) -I$(top_srcdir)/digikam/libs/dimg \
	                       -I$(top_srcdir)/digikam/digikam 

digikaminclude_HEADERS = bcgmodifier.h hslmodifier.h 
digikamincludedir = $(includedir)/digikam
