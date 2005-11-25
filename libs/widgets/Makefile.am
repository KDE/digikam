METASOURCES = AUTO

noinst_LTLIBRARIES = libwidgets.la

libwidgets_la_SOURCES = imageregionwidget.cpp imagepaniconwidget.cpp \
			            histogramwidget.cpp colorgradientwidget.cpp imageselectionwidget.cpp \
			            imageguidewidget.cpp curveswidget.cpp imagepannelwidget.cpp \
			            sidebar.cpp

libwidgets_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = -I$(top_srcdir)/digikam/libs/histogram \
	       -I$(top_srcdir)/digikam/libs/curves \
	       -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/digikam \
	       -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
	       $(all_includes)

digikaminclude_HEADERS = imageregionwidget.h imagepaniconwidget.h \
			             histogramwidget.h colorgradientwidget.h imageselectionwidget.h \
			             imageguidewidget.h curveswidget.h imagepannelwidget.h \
			             sidebar.h

digikamincludedir = $(includedir)/digikam
