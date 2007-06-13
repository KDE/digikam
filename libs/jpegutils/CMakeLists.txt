INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(jpegutils_STAT_SRCS
   jpegutils.cpp
   transupp.cpp
   )

KDE3_AUTOMOC(${jpegutils_STAT_SRCS})

ADD_LIBRARY(jpegutils STATIC ${jpegutils_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#METASOURCES = AUTO
#
## --enable-final triggers: http://bugs.kde.org/show_bug.cgi?id=126326
## digikam: camera download: auto-rotated images lose EXIF info ...
## So make sure nofinal is always used here!
#KDE_OPTIONS = nofinal
#
#INCLUDES = $(all_includes) \
#	   -I$(top_srcdir)/digikam/libs/dmetadata \
#	   -I$(top_srcdir)/digikam/libs/dimg \
#	   $(LIBKEXIV2_CFLAGS) \
#	   -I$(top_srcdir)/digikam/digikam
#
#noinst_LTLIBRARIES = libjpegutils.la
#
#libjpegutils_la_SOURCES = jpegutils.cpp transupp.cpp
#
#libjpegutils_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#libjpegutils_la_LIBADD  = $(LIBJPEG) 
