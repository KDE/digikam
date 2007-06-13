INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/themeengine ${CMAKE_SOURCE_DIR}/digikam/libs/threadimageio ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(slideshow_STAT_SRCS
   toolbar.cpp
   slideshow.cpp
   )

KDE3_AUTOMOC(${slideshow_STAT_SRCS})

ADD_LIBRARY(slideshow STATIC ${slideshow_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#INCLUDES = -I$(top_srcdir)/digikam/digikam \
#	   -I$(top_srcdir)/digikam/libs/dimg \
#	   -I$(top_srcdir)/digikam/libs/dmetadata \
#	   -I$(top_srcdir)/digikam/libs/themeengine \
#	   -I$(top_srcdir)/digikam/libs/threadimageio \
#	   $(all_includes)
#
#noinst_LTLIBRARIES = libslideshow.la 
#
#libslideshow_la_SOURCES = toolbar.cpp slideshow.cpp 
#
#libslideshow_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#
