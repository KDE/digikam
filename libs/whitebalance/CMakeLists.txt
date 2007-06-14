
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/histogram
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dimg
                     ${CMAKE_SOURCE_DIR}/digikam/digikam 
                     ${KDE4_INCLUDE_DIR} 
                     ${QT_INCLUDES}  
                   )

SET(whitebalance_STAT_SRCS whitebalance.cpp )

KDE4_ADD_LIBRARY(whitebalance STATIC ${whitebalance_STAT_SRCS})

INSTALL( FILES whitebalance.h DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )


#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libwhitebalance.la
#
#libwhitebalance_la_SOURCES = whitebalance.cpp 
#
#libwhitebalance_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#INCLUDES = -I$(top_srcdir)/digikam/libs/histogram \
#	   -I$(top_srcdir)/digikam/libs/dimg \
#	   -I$(top_srcdir)/digikam/digikam \
#	   $(all_includes) 
#
#digikaminclude_HEADERS = whitebalance.h 
#
#digikamincludedir = $(includedir)/digikam
