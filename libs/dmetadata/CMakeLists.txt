INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(dmetadata_STAT_SRCS
   dmetadata.cpp
   )

KDE3_AUTOMOC(${dmetadata_STAT_SRCS})

ADD_LIBRARY(dmetadata STATIC ${dmetadata_STAT_SRCS})


########### install files ###############

INSTALL_FILES( /include/digikam FILES  dmetadata.h photoinfocontainer.h  )



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libdmetadata.la
#
#libdmetadata_la_SOURCES = dmetadata.cpp
#
#libdmetadata_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#libdmetadata_la_LIBADD  = $(LIBKEXIV2_LIBS) $(LIBKDCRAW_LIBS) 
#	                 
#INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
#	   -I$(top_srcdir)/digikam/digikam \
#	   $(LIBKEXIV2_CFLAGS) $(LIBKDCRAW_CFLAGS) $(all_includes)
#
#digikaminclude_HEADERS = dmetadata.h photoinfocontainer.h 
#digikamincludedir = $(includedir)/digikam
