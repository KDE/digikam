METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/digikam/digikam \
	   -I$(top_srcdir)/digikam/libs/dimg \
	   -I$(top_srcdir)/digikam/libs/dmetadata \
	   -I$(top_srcdir)/digikam/libs/themeengine \
	   -I$(top_srcdir)/digikam/libs/threadimageio \
	   $(all_includes)

noinst_LTLIBRARIES = libslideshow.la 

libslideshow_la_SOURCES = toolbar.cpp slideshow.cpp 

libslideshow_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)


