
include_directories( ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/themeengine ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters ${CMAKE_SOURCE_DIR}/digikam/libs/imageproperties ${CMAKE_SOURCE_DIR}/digikam/libs/threadimageio ${CMAKE_SOURCE_DIR}/digikam/libs/database ${CMAKE_SOURCE_DIR}/digikam/utilities/setup ${CMAKE_SOURCE_DIR}/digikam/utilities/slideshow ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/tools ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############


########### next target ###############


########### install files ###############

install( FILES  imageiface.h  DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )
install( FILES  digikamimageplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
install( FILES  digikamimagewindowui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam )



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libdimgeditor.la libshowfoto.la 
#
#libdimgeditor_la_SOURCES = editorwindow.cpp imageiface.cpp imagewindow.cpp   
#
#libdimgeditor_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_KDEPRINT) 
#
#libdimgeditor_la_LIBADD = $(top_builddir)/digikam/utilities/imageeditor/tools/libdimgeditortools.la 
#
#libshowfoto_la_SOURCES = editorwindow.cpp imageiface.cpp 
#
#libshowfoto_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_KDEPRINT) 
#
#libshowfoto_la_LIBADD = $(top_builddir)/digikam/libs/dimg/libdimg.la \
#	                $(top_builddir)/digikam/utilities/imageeditor/canvas/libdimgcanvas.la \
#	                $(top_builddir)/digikam/libs/greycstoration/libgreycstoration.la \
#	                $(top_builddir)/digikam/utilities/imageeditor/tools/libdimgeditortools.la \
#	                $(top_builddir)/digikam/libs/widgets/libwidgets.la
#
#INCLUDES = -I$(top_srcdir)/digikam/digikam \
#	   -I$(top_srcdir)/digikam/libs/widgets/common \
#	   -I$(top_srcdir)/digikam/libs/dialogs \
#	   -I$(top_srcdir)/digikam/libs/dimg \
#	   -I$(top_srcdir)/digikam/libs/themeengine \
#	   -I$(top_srcdir)/digikam/libs/dmetadata \
#	   -I$(top_srcdir)/digikam/libs/dimg/filters \
#	   -I$(top_srcdir)/digikam/libs/imageproperties \
#	   -I$(top_srcdir)/digikam/libs/threadimageio \
#	   -I$(top_srcdir)/digikam/libs/database \
#	   -I$(top_srcdir)/digikam/utilities/setup \
#	   -I$(top_srcdir)/digikam/utilities/slideshow \
#	   -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
#	   -I$(top_srcdir)/digikam/utilities/imageeditor/tools \
#	   -I$(top_builddir)/digikam/libs/dialogs \
#	   $(LIBKEXIV2_CFLAGS) $(LIBKDCRAW_CFLAGS) $(all_includes) 
#
#digikaminclude_HEADERS = imageiface.h 
#
#digikamincludedir = $(includedir)/digikam
#
#rcdir = $(kde_datadir)/digikam
#rc_DATA = digikamimagewindowui.rc
#
#kde_servicetypes_DATA = digikamimageplugin.desktop
#
