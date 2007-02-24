METASOURCES = AUTO
KDE_CXXFLAGS = $(USE_EXCEPTIONS)

noinst_LTLIBRARIES = libmetadatawidgets.la

libmetadatawidgets_la_SOURCES = metadatalistview.cpp metadatalistviewitem.cpp metadatawidget.cpp \
	                            iptcwidget.cpp exifwidget.cpp mdkeylistviewitem.cpp makernotewidget.cpp \
	                            gpswidget.cpp worldmapwidget.cpp

libmetadatawidgets_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
                 
INCLUDES = -I$(top_srcdir)/digikam/libs/dmetadata \
	       -I$(top_srcdir)/digikam/libs/dimg \
	       $(LIBKEXIV2_CFLAGS) \
	       -I$(top_srcdir)/digikam/digikam \
	       $(all_includes)
