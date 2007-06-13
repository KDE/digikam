INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor ${CMAKE_SOURCE_DIR}/digikam/libs/levels ${CMAKE_SOURCE_DIR}/digikam/libs/histogram ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(dimgfilters_STAT_SRCS
   bcgmodifier.cpp
   hslmodifier.cpp
   icctransform.cpp
   dimgimagefilters.cpp
   dimgthreadedfilter.cpp
   dimggaussianblur.cpp
   dimgsharpen.cpp
   colormodifier.cpp
   )

KDE3_AUTOMOC(${dimgfilters_STAT_SRCS})

ADD_LIBRARY(dimgfilters STATIC ${dimgfilters_STAT_SRCS})


########### install files ###############

INSTALL_FILES( /include/digikam FILES  bcgmodifier.h hslmodifier.h dimgthreadedfilter.h dimgimagefilters.h  	                 icctransform.h colormodifier.h dimgsharpen.h dimggaussianblur.h  )



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libdimgfilters.la
#
#libdimgfilters_la_SOURCES = bcgmodifier.cpp hslmodifier.cpp icctransform.cpp \
#	                    dimgimagefilters.cpp dimgthreadedfilter.cpp \
#	                    dimggaussianblur.cpp dimgsharpen.cpp colormodifier.cpp 
#
#libdimgfilters_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
#	       -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
#	       -I$(top_srcdir)/digikam/libs/levels \
#	       -I$(top_srcdir)/digikam/libs/histogram \
#	       -I$(top_srcdir)/digikam/digikam \
#	       $(all_includes) 
#	   
#
#digikaminclude_HEADERS = bcgmodifier.h hslmodifier.h dimgthreadedfilter.h dimgimagefilters.h \
#	                 icctransform.h colormodifier.h dimgsharpen.h dimggaussianblur.h 
#digikamincludedir = $(includedir)/digikam
