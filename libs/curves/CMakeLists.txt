
include_directories( ${CMAKE_SOURCE_DIR}/digikam/libs/histogram 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg 
		     ${CMAKE_SOURCE_DIR}/digikam/digikam 
		     ${KDE4_INCLUDE_DIR} 
		     ${QT_INCLUDES}
                   )

SET(curves_STAT_SRCS imagecurves.cpp )

KDE4_ADD_LIBRARY(curves STATIC ${curves_STAT_SRCS})

install( FILES imagecurves.h DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )



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
