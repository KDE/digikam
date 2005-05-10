METASOURCES = AUTO

noinst_LTLIBRARIES = liblevels.la

liblevels_la_SOURCES = imagelevels.cpp 

liblevels_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = \
	-I$(top_srcdir)/digikam/libs/histogram \
	-I$(top_srcdir)/digikam/ \
	-I$(top_srcdir)/digikam/digikam \
	$(all_includes) 

digikaminclude_HEADERS = imagelevels.h 

digikamincludedir = $(includedir)/digikam
