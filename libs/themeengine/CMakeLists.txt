INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/ ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(themeengine_STAT_SRCS
   theme.cpp
   themeengine.cpp
   texture.cpp
   )

KDE3_AUTOMOC(${themeengine_STAT_SRCS})

ADD_LIBRARY(themeengine STATIC ${themeengine_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libthemeengine.la
#
#libthemeengine_la_SOURCES = theme.cpp themeengine.cpp texture.cpp
#
#libthemeengine_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
# 	       -I$(top_srcdir)/digikam/digikam \
#	       -I$(top_srcdir)/digikam/ \
#	       $(all_includes)