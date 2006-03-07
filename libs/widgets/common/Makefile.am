METASOURCES = AUTO

noinst_LTLIBRARIES = libcommonwidgets.la

libcommonwidgets_la_SOURCES = histogramwidget.cpp colorgradientwidget.cpp curveswidget.cpp \
	                          sidebar.cpp cietonguewidget.cpp iccpreviewwidget.cpp

libcommonwidgets_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

libcommonwidgets_la_LIBADD = $(top_builddir)/digikam/libs/lprof/liblprof.la 
                 
INCLUDES = -I$(top_srcdir)/digikam/libs/histogram \
	       -I$(top_srcdir)/digikam/libs/curves \
	       -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/libs/lprof \
	       -I$(top_srcdir)/digikam/digikam \
	       $(all_includes)

digikaminclude_HEADERS = histogramwidget.h colorgradientwidget.h sidebar.h cietonguewidget.h \
	                     iccpreviewwidget.h curveswidget.h 

digikamincludedir = $(includedir)/digikam
