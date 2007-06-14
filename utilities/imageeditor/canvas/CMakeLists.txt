
include_directories( ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs ${CMAKE_SOURCE_DIR}/digikam/libs/histogram ${CMAKE_SOURCE_DIR}/digikam/libs/threadimageio ${CMAKE_SOURCE_DIR}/digikam/utilities/splashscreen ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############


########### install files ###############

install( FILES  imageplugin.h  DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libdimgcanvas.la
#
#libdimgcanvas_la_SOURCES = dimginterface.cpp colorcorrectiondlg.cpp \
#	                       canvas.cpp undocache.cpp \
#	                       undoaction.cpp undomanager.cpp \
#	                       imagepluginloader.cpp imageplugin.cpp
#
#libdimgcanvas_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TIFF) 
#
#INCLUDES = -I$(top_srcdir)/digikam/digikam \
#	       -I$(top_srcdir)/digikam/libs/dimg \
#	       -I$(top_srcdir)/digikam/libs/dimg/filters \
#	       -I$(top_srcdir)/digikam/libs/dmetadata \
#	       -I$(top_srcdir)/digikam/libs/dialogs \
#	       -I$(top_srcdir)/digikam/libs/histogram \
#	       -I$(top_srcdir)/digikam/libs/threadimageio \
#	       -I$(top_srcdir)/digikam/utilities/splashscreen \
#	       -I$(top_srcdir)/digikam/libs/widgets/imageplugins \
#	       -I$(top_srcdir)/digikam/libs/widgets/common \
#	       $(LIBKEXIV2_CFLAGS) $(LIBKDCRAW_CFLAGS) $(all_includes) 
#
#digikaminclude_HEADERS = imageplugin.h 
#
#digikamincludedir = $(includedir)/digikam
#
