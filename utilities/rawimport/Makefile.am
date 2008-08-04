METASOURCES = AUTO

noinst_LTLIBRARIES = librawimport.la 

librawimport_la_SOURCES = rawpreview.cpp rawimportdlg.cpp

librawimport_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_KDEPRINT) 

INCLUDES= -I$(top_srcdir)/digikam/digikam \
	  -I$(top_srcdir)/digikam/libs/dmetadata \
	  -I$(top_srcdir)/digikam/libs/dimg \
	  -I$(top_srcdir)/digikam/libs/dimg/filters \
	  -I$(top_srcdir)/digikam/libs/widgets/common \
	  -I$(top_srcdir)/digikam/libs/threadimageio \
	  -I$(top_srcdir)/digikam/libs/themeengine \
	  $(LIBKDCRAW_CFLAGS) \
	  $(all_includes) 

