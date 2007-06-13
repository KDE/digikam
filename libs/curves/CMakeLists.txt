INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/histogram ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(curves_STAT_SRCS
   imagecurves.cpp
   )

KDE3_AUTOMOC(${curves_STAT_SRCS})

ADD_LIBRARY(curves STATIC ${curves_STAT_SRCS})


########### install files ###############

INSTALL_FILES( /include/digikam FILES  imagecurves.h  )



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libcurves.la
#
#libcurves_la_SOURCES = imagecurves.cpp 
#
#libcurves_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#INCLUDES = -I$(top_srcdir)/digikam/libs/histogram \
#	       -I$(top_srcdir)/digikam/libs/dimg \
#	       -I$(top_srcdir)/digikam/digikam \
#	       $(all_includes) 
#
#digikaminclude_HEADERS = imagecurves.h 
#
#digikamincludedir = $(includedir)/digikam
