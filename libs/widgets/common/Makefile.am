METASOURCES = AUTO

noinst_LTLIBRARIES = libcommonwidgets.la

libcommonwidgets_la_SOURCES = histogramwidget.cpp colorgradientwidget.cpp curveswidget.cpp \
	                          sidebar.cpp 

libcommonwidgets_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = -I$(top_srcdir)/digikam/libs/histogram \
	       -I$(top_srcdir)/digikam/libs/curves \
	       -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/digikam \
	       $(all_includes)

digikaminclude_HEADERS = histogramwidget.h colorgradientwidget.h sidebar.h curveswidget.h 

digikamincludedir = $(includedir)/digikam
