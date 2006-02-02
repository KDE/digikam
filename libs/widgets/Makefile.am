METASOURCES = AUTO

noinst_LTLIBRARIES = libwidgets.la

libwidgets_la_SOURCES = imageregionwidget.cpp imagepaniconwidget.cpp \
	                histogramwidget.cpp colorgradientwidget.cpp \
	                imageselectionwidget.cpp imageguidewidget.cpp \
	                curveswidget.cpp imagepannelwidget.cpp imagewidget.cpp \
	                sidebar.cpp cietonguewidget.cpp iccpreviewwidget.cpp 

libwidgets_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

libwidgets_la_LIBADD = $(top_builddir)/digikam/libs/lprof/liblprof.la 

INCLUDES = -I$(top_srcdir)/digikam/libs/histogram \
	       -I$(top_srcdir)/digikam/libs/curves \
	       -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/libs/lprof \
	       -I$(top_srcdir)/digikam/digikam \
	       -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
	       $(all_includes)

digikaminclude_HEADERS = imageregionwidget.h imagepaniconwidget.h imagewidget.h \
	                 histogramwidget.h colorgradientwidget.h imageselectionwidget.h \
	                 curveswidget.h imagepannelwidget.h sidebar.h cietonguewidget.h \
	                 imageguidewidget.h iccpreviewwidget.h 

digikamincludedir = $(includedir)/digikam
