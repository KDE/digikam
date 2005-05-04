METASOURCES = AUTO

noinst_LTLIBRARIES = libcurves.la

libcurves_la_SOURCES = imagecurves.cpp 

libcurves_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = \
	-I$(top_srcdir)/digikam/libs/histogram \
	$(all_includes) 

digikaminclude_HEADERS = imagecurves.h 

digikamincludedir = $(includedir)/digikam
