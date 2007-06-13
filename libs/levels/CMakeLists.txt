INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/histogram ${CMAKE_SOURCE_DIR}/digikam/ ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(levels_STAT_SRCS
   imagelevels.cpp
   )

KDE3_AUTOMOC(${levels_STAT_SRCS})

ADD_LIBRARY(levels STATIC ${levels_STAT_SRCS})


########### install files ###############

INSTALL_FILES( /include/digikam FILES  imagelevels.h  )



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
