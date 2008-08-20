METASOURCES = AUTO

noinst_LTLIBRARIES = libdimgeditortools.la 

libdimgeditortools_la_SOURCES = imageresize.cpp imageprint.cpp

libdimgeditortools_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_KDEPRINT) 

INCLUDES= -I$(top_srcdir)/digikam/digikam \
	  -I$(top_srcdir)/digikam/libs/dimg \
	  -I$(top_srcdir)/digikam/libs/dimg/filters \
	  -I$(top_srcdir)/digikam/libs/dmetadata \
	  -I$(top_srcdir)/digikam/libs/widgets/common \
	  -I$(top_srcdir)/digikam/libs/greycstoration \
	  -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
	  -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
	  $(LIBKDCRAW_CFLAGS) \
	  $(all_includes) 

