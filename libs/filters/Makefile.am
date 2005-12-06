METASOURCES = AUTO

noinst_LTLIBRARIES = libfilters.la

libfilters_la_SOURCES = imagefilters.cpp threadedfilter.cpp gaussianblur.cpp sharpen.cpp 

libfilters_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
	       -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/libs/levels \
	       -I$(top_srcdir)/digikam/libs/histogram \
	       -I$(top_srcdir)/digikam/digikam \
	       $(all_includes) 

digikaminclude_HEADERS = imagefilters.h threadedfilter.h

digikamincludedir = $(includedir)/digikam
