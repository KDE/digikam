
include_directories( ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/digikam ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############


########### install files ###############




#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libmetadatawidgets.la
#
#libmetadatawidgets_la_SOURCES = metadatalistview.cpp metadatalistviewitem.cpp metadatawidget.cpp \
#	                            iptcwidget.cpp exifwidget.cpp mdkeylistviewitem.cpp \
#	                            makernotewidget.cpp gpswidget.cpp worldmapwidget.cpp
#
#libmetadatawidgets_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#                 
#INCLUDES = -I$(top_srcdir)/digikam/libs/dmetadata \
#	       -I$(top_srcdir)/digikam/libs/dimg \
#	       -I$(top_srcdir)/digikam/digikam \
#	       $(LIBKEXIV2_CFLAGS) \
#	       $(all_includes)
