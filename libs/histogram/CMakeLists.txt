INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(histogram_STAT_SRCS
   imagehistogram.cpp
   )

KDE3_AUTOMOC(${histogram_STAT_SRCS})

ADD_LIBRARY(histogram STATIC ${histogram_STAT_SRCS})


########### install files ###############

INSTALL_FILES( /include/digikam FILES  imagehistogram.h  )



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libhistogram.la
#
#libhistogram_la_SOURCES = imagehistogram.cpp  
#
#libhistogram_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
#	       -I$(top_srcdir)/digikam/digikam \
#	       $(all_includes) 
#
#digikaminclude_HEADERS = imagehistogram.h 
#
#digikamincludedir = $(includedir)/digikam
