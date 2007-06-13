INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(thumbbar_STAT_SRCS
   thumbbar.cpp
   thumbnailjob.cpp
   )

KDE3_AUTOMOC(${thumbbar_STAT_SRCS})

ADD_LIBRARY(thumbbar STATIC ${thumbbar_STAT_SRCS})


########### install files ###############

INSTALL_FILES( /include/digikam FILES  thumbbar.h  )



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libthumbbar.la
#
#libthumbbar_la_SOURCES = thumbbar.cpp thumbnailjob.cpp
#
#libthumbbar_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
#	       -I$(top_srcdir)/digikam/libs/dmetadata \
#	       -I$(top_srcdir)/digikam/digikam \
#	       $(all_includes)
#
#digikaminclude_HEADERS = thumbbar.h 
#
#digikamincludedir = $(includedir)/digikam
