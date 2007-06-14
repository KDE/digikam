
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg 
                     ${CMAKE_SOURCE_DIR}/digikam/digikam 
                     ${KDE4_INCLUDE_DIR} 
                     ${QT_INCLUDES}  
                   )

SET(histogram_STAT_SRCS imagehistogram.cpp )

KDE4_ADD_LIBRARY(histogram STATIC ${histogram_STAT_SRCS})

INSTALL( FILES imagehistogram.h DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )


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
