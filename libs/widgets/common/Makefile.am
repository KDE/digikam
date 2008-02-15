METASOURCES = AUTO

noinst_LTLIBRARIES = libcommonwidgets.la

libcommonwidgets_la_SOURCES = histogramwidget.cpp colorgradientwidget.cpp curveswidget.cpp dlogoaction.cpp \
                              sidebar.cpp squeezedcombobox.cpp filesaveoptionsbox.cpp dpopupmenu.cpp \
                              statuszoombar.cpp statusnavigatebar.cpp statusprogressbar.cpp searchtextbar.cpp \
                              dcursortracker.cpp paniconwidget.cpp previewwidget.cpp splashscreen.cpp statusled.cpp 

libcommonwidgets_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = -I$(top_srcdir)/digikam/libs/histogram \
	       -I$(top_srcdir)/digikam/libs/curves \
	       -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/libs/dimg/loaders \
	       -I$(top_srcdir)/digikam/digikam \
	       $(LIBKDCRAW_CFLAGS) \
	       $(all_includes)


digikaminclude_HEADERS = histogramwidget.h colorgradientwidget.h curveswidget.h sidebar.h dlogoaction.h \
	                     squeezedcombobox.h dpopupmenu.h statuszoombar.h statusnavigatebar.h searchtextbar.h \
	                     statusprogressbar.h dcursortracker.h paniconwidget.h previewwidget.h statusled.h 

digikamincludedir = $(includedir)/digikam
