INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/histogram ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(whitebalance_STAT_SRCS
   whitebalance.cpp
   )

KDE3_AUTOMOC(${whitebalance_STAT_SRCS})

ADD_LIBRARY(whitebalance STATIC ${whitebalance_STAT_SRCS})


########### install files ###############

INSTALL_FILES( /include/digikam FILES  whitebalance.h  )



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
