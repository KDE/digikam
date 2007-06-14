
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg 
                     ${CMAKE_SOURCE_DIR}/digikam/libs/histogram 
                     ${CMAKE_SOURCE_DIR}/digikam/digikam 
                     ${KDE4_INCLUDE_DIR}
                     ${QT_INCLUDES}  
                   )

SET(levels_STAT_SRCS imagelevels.cpp )

KDE4_ADD_LIBRARY(levels STATIC ${levels_STAT_SRCS})

INSTALL( FILES imagelevels.h DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )


#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = liblevels.la
#
#liblevels_la_SOURCES = imagelevels.cpp 
#
#liblevels_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
#	       -I$(top_srcdir)/digikam/libs/histogram \
#	       -I$(top_srcdir)/digikam/ \
#	       -I$(top_srcdir)/digikam/digikam \
#	       $(all_includes)
#
#digikaminclude_HEADERS = imagelevels.h 
#
#digikamincludedir = $(includedir)/digikam
