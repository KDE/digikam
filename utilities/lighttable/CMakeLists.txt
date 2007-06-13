INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs ${CMAKE_SOURCE_DIR}/digikam/libs/thumbbar ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/database ${CMAKE_SOURCE_DIR}/digikam/libs/themeengine ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters ${CMAKE_SOURCE_DIR}/digikam/libs/imageproperties ${CMAKE_SOURCE_DIR}/digikam/libs/threadimageio ${CMAKE_SOURCE_DIR}/digikam/utilities/setup ${CMAKE_SOURCE_DIR}/digikam/utilities/slideshow ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(lighttable_STAT_SRCS
   lighttablebar.cpp
   lighttablewindow.cpp
   lighttablepreview.cpp
   lighttableview.cpp
   )

KDE3_AUTOMOC(${lighttable_STAT_SRCS})

ADD_LIBRARY(lighttable STATIC ${lighttable_STAT_SRCS})


########### install files ###############

INSTALL_FILES( /share/apps/digikam FILES  lighttablewindowui.rc )



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#INCLUDES = -I$(top_srcdir)/digikam/digikam \
#	       -I$(top_srcdir)/digikam/libs/widgets/common \
#	       -I$(top_srcdir)/digikam/libs/dialogs \
#	       -I$(top_srcdir)/digikam/libs/thumbbar \
#	       -I$(top_srcdir)/digikam/libs/dimg \
#	       -I$(top_srcdir)/digikam/libs/database \
#	       -I$(top_srcdir)/digikam/libs/themeengine \
#	       -I$(top_srcdir)/digikam/libs/dmetadata \
#	       -I$(top_srcdir)/digikam/libs/dimg/filters \
#	       -I$(top_srcdir)/digikam/libs/imageproperties \
#	       -I$(top_srcdir)/digikam/libs/threadimageio \
#	       -I$(top_srcdir)/digikam/utilities/setup \
#	       -I$(top_srcdir)/digikam/utilities/slideshow \
#	       -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
#	       -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
#	       -I$(top_builddir)/digikam/libs/dialogs \
#	       $(LIBKEXIV2_CFLAGS) $(LIBKDCRAW_CFLAGS) $(all_includes) 
#
#noinst_LTLIBRARIES = liblighttable.la 
#
#liblighttable_la_SOURCES = lighttablebar.cpp lighttablewindow.cpp lighttablepreview.cpp \
#                           lighttableview.cpp 
#
#liblighttable_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#rcdir = $(kde_datadir)/digikam
#rc_DATA = lighttablewindowui.rc
