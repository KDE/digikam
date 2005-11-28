METASOURCES = AUTO

noinst_LTLIBRARIES = libhistogram.la

libhistogram_la_SOURCES = imagehistogram.cpp  

libhistogram_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/digikam \
	       $(all_includes) 

digikaminclude_HEADERS = imagehistogram.h 

digikamincludedir = $(includedir)/digikam
