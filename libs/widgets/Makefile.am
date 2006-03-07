SUBDIRS = metadata imageplugins 
METASOURCES = AUTO

noinst_LTLIBRARIES = libwidgets.la

libwidgets_la_SOURCES = histogramwidget.cpp colorgradientwidget.cpp curveswidget.cpp \
	                    sidebar.cpp cietonguewidget.cpp iccpreviewwidget.cpp

libwidgets_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

libwidgets_la_LIBADD = $(top_builddir)/digikam/libs/lprof/liblprof.la \
		       $(top_builddir)/digikam/libs/widgets/metadata/libmetadatawidgets.la \
		       $(top_builddir)/digikam/libs/widgets/imageplugins/libimagepluginswidgets.la 

INCLUDES = -I$(top_srcdir)/digikam/libs/histogram \
	       -I$(top_srcdir)/digikam/libs/curves \
	       -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/libs/lprof \
	       -I$(top_srcdir)/digikam/digikam \
	       $(all_includes)

digikaminclude_HEADERS = histogramwidget.h colorgradientwidget.h sidebar.h cietonguewidget.h \
	                     iccpreviewwidget.h curveswidget.h 

digikamincludedir = $(includedir)/digikam
