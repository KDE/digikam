METASOURCES = AUTO

KDE_CXXFLAGS = $(USE_EXCEPTIONS) -w 

noinst_LTLIBRARIES = libgreycstoration.la

libgreycstoration_la_SOURCES = greycstorationiface.cpp greycstorationwidget.cpp

libgreycstoration_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/libs/dimg/filters \
	       -I$(top_srcdir)/digikam/digikam \
		$(LIBKDCRAW_CFLAGS) \
		$(all_includes)

digikaminclude_HEADERS = greycstorationiface.h greycstorationwidget.h greycstorationsettings.h 

digikamincludedir = $(includedir)/digikam

