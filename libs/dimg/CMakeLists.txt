ADD_SUBDIRECTORY( loaders ) 
ADD_SUBDIRECTORY( filters ) 

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/loaders ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(dimg_STAT_SRCS
   dimg.cpp
   dimgscale.cpp
   dcolor.cpp
   dcolorcomposer.cpp
   ddebug.cpp
   fastscale.cpp
   )

KDE3_AUTOMOC(${dimg_STAT_SRCS})

ADD_LIBRARY(dimg STATIC ${dimg_STAT_SRCS})


########### install files ###############

INSTALL_FILES( /include/digikam FILES  dimg.h dcolor.h dcolorpixelaccess.h dcolorcomposer.h                           dcolorblend.h ddebug.h )



#original Makefile.am contents follow:

#SUBDIRS = loaders filters 
#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libdimg.la
#
#libdimg_la_SOURCES = dimg.cpp dimgscale.cpp dcolor.cpp dcolorcomposer.cpp ddebug.cpp fastscale.cpp
#
#libdimg_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
#
#libdimg_la_LIBADD  = $(top_builddir)/digikam/libs/histogram/libhistogram.la \
#	             $(top_builddir)/digikam/libs/levels/liblevels.la \
#	             $(top_builddir)/digikam/libs/curves/libcurves.la \
#	             $(top_builddir)/digikam/libs/whitebalance/libwhitebalance.la \
#	             $(top_builddir)/digikam/libs/dimg/loaders/libdimgloaders.la \
#	             $(top_builddir)/digikam/libs/dimg/filters/libdimgfilters.la \
#	             $(top_builddir)/digikam/libs/dmetadata/libdmetadata.la \
#	             $(LIBKDCRAW_LIBS) $(LCMS_LIBS) 
#
#INCLUDES = $(all_includes) $(LIBKDCRAW_CFLAGS) \
#	   -I$(top_srcdir)/digikam/libs/dimg/loaders \
#	   -I$(top_srcdir)/digikam/libs/dimg/filters \
#	   -I$(top_srcdir)/digikam/libs/dmetadata \
#	   -I$(top_srcdir)/digikam/digikam
#
#digikaminclude_HEADERS = dimg.h dcolor.h dcolorpixelaccess.h dcolorcomposer.h \
#                         dcolorblend.h ddebug.h
#digikamincludedir = $(includedir)/digikam
