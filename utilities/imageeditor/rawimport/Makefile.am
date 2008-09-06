METASOURCES = AUTO

noinst_LTLIBRARIES = librawimport.la 

librawimport_la_SOURCES = rawpreview.cpp rawsettingsbox.cpp rawimport.cpp \
	                  rawpostprocessing.cpp 

librawimport_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_KDEPRINT) 

INCLUDES= -I$(top_srcdir)/digikam/digikam \
	  -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
	  -I$(top_srcdir)/digikam/libs/histogram \
	  -I$(top_srcdir)/digikam/libs/curves \
	  -I$(top_srcdir)/digikam/libs/levels \
	  -I$(top_srcdir)/digikam/libs/whitebalance \
	  -I$(top_srcdir)/digikam/libs/dmetadata \
	  -I$(top_srcdir)/digikam/libs/curves \
	  -I$(top_srcdir)/digikam/libs/dimg \
	  -I$(top_srcdir)/digikam/libs/dimg/filters \
	  -I$(top_srcdir)/digikam/libs/dialogs \
	  -I$(top_srcdir)/digikam/libs/widgets/common \
	  -I$(top_srcdir)/digikam/libs/widgets/iccprofiles \
	  -I$(top_srcdir)/digikam/libs/threadimageio \
	  -I$(top_srcdir)/digikam/libs/themeengine \
	  $(LIBKDCRAW_CFLAGS) \
	  $(all_includes) 

