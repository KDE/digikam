METASOURCES = AUTO

noinst_LTLIBRARIES = libwhitebalance.la

libwhitebalance_la_SOURCES = whitebalance.cpp 

libwhitebalance_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = -I$(top_srcdir)/digikam/libs/histogram \
	   -I$(top_srcdir)/digikam/libs/dimg \
	   -I$(top_srcdir)/digikam/digikam \
	   $(all_includes) 

digikaminclude_HEADERS = whitebalance.h 

digikamincludedir = $(includedir)/digikam
