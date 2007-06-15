
INCLUDE_DIRECTORIES(
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata 
                   )

SET(libdimgloaders_STAT_SRCS dimgloader.cpp pngloader.cpp jpegloader.cpp
                             tiffloader.cpp rawloader.cpp ppmloader.cpp
                             qimageloader.cpp iccjpeg.c jp2kloader.cpp
                             jpegsettings.cpp pngsettings.cpp tiffsettings.cpp
                             jp2ksettings.cpp )

KDE4_ADD_LIBRARY(libdimgloaders STATIC ${libdimgloaders_STAT_SRCS})

TARGET_LINK_LIBRARIES(libdimgloaders
                      ${JPEG_LIBRARIES} 
                      ${TIFF_LIBRARIES} 
                      ${PNG_LIBRARIES} 
                      ${JASPER_LIBRARIES}
                     )

INSTALL( FILES imagelevels.h DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )


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
