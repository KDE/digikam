METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/digikam/digikam \
	       -I$(top_srcdir)/digikam/libs/widgets/common \
	       -I$(top_srcdir)/digikam/libs/dialogs \
	       -I$(top_srcdir)/digikam/libs/thumbbar \
	       -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/libs/themeengine \
	       -I$(top_srcdir)/digikam/libs/dmetadata \
	       -I$(top_srcdir)/digikam/libs/dimg/filters \
	       -I$(top_srcdir)/digikam/libs/imageproperties \
	       -I$(top_srcdir)/digikam/libs/threadimageio \
	       -I$(top_srcdir)/digikam/utilities/setup \
	       -I$(top_srcdir)/digikam/utilities/slideshow \
	       -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
	       -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
	       -I$(top_builddir)/digikam/libs/dialogs \
	       $(LIBKEXIV2_CFLAGS) $(LIBKDCRAW_CFLAGS) $(all_includes) 

noinst_LTLIBRARIES = liblighttable.la 

liblighttable_la_SOURCES = lighttablebar.cpp lighttablewindow.cpp lighttablepreview.cpp \
                           lighttableview.cpp 

liblighttable_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

rcdir = $(kde_datadir)/digikam
rc_DATA = lighttablewindowui.rc
