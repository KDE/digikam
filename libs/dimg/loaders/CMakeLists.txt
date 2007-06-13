INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/dcraw ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(dimgloaders_STAT_SRCS
   dimgloader.cpp
   pngloader.cpp
   jpegloader.cpp
   tiffloader.cpp
   rawloader.cpp
   ppmloader.cpp
   qimageloader.cpp
   iccjpeg.c
   jp2kloader.cpp
   jpegsettings.cpp
   pngsettings.cpp
   tiffsettings.cpp
   jp2ksettings.cpp
   )

KDE3_AUTOMOC(${dimgloaders_STAT_SRCS})

ADD_LIBRARY(dimgloaders STATIC ${dimgloaders_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#METASOURCES = AUTO
#KDE_CXXFLAGS = $(USE_EXCEPTIONS)
#
#noinst_LTLIBRARIES = libdimgloaders.la
#
#libdimgloaders_la_SOURCES = dimgloader.cpp pngloader.cpp jpegloader.cpp tiffloader.cpp \
#	                    rawloader.cpp ppmloader.cpp qimageloader.cpp iccjpeg.c \
#	                    jp2kloader.cpp jpegsettings.cpp pngsettings.cpp \
#	                    tiffsettings.cpp jp2ksettings.cpp 
#
#libdimgloaders_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) \
#	                    $(LIBJPEG) $(LIB_TIFF) $(LIB_PNG) $(LIB_JASPER)
#
#INCLUDES = $(all_includes) -I$(top_srcdir)/digikam/libs/dimg \
#	                   -I$(top_srcdir)/digikam/libs/dmetadata \
#	                   -I$(top_srcdir)/digikam/libs/dcraw \
#	                   $(LIBKEXIV2_CFLAGS) $(LIBKDCRAW_CFLAGS) \
#	                   -I$(top_srcdir)/digikam/digikam 
