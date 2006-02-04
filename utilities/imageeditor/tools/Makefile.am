METASOURCES = AUTO

noinst_LTLIBRARIES = libdimgeditortools.la 

libdimgeditortools_la_SOURCES = imageresizedlg.cpp slideshow.cpp imageprint.cpp

libdimgeditortools_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_KDEPRINT) 

INCLUDES= -I$(top_srcdir)/digikam/digikam \
	  -I$(top_srcdir)/digikam/libs/dimg \
	  -I$(top_srcdir)/digikam/libs/dimg/filters \
	  -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
	  -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
	  $(all_includes) 

