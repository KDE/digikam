INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(greycstoration_STAT_SRCS
   greycstorationiface.cpp
   greycstorationwidget.cpp
   )

KDE3_AUTOMOC(${greycstoration_STAT_SRCS})

ADD_LIBRARY(greycstoration STATIC ${greycstoration_STAT_SRCS})


########### install files ###############

INSTALL_FILES( /include/digikam FILES  greycstorationiface.h greycstorationwidget.h greycstorationsettings.h  )



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#KDE_CXXFLAGS = $(USE_EXCEPTIONS) -w 
#
#noinst_LTLIBRARIES = libgreycstoration.la
#
#libgreycstoration_la_SOURCES = greycstorationiface.cpp greycstorationwidget.cpp
#
#libgreycstoration_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
#	       -I$(top_srcdir)/digikam/libs/dimg/filters \
#	       -I$(top_srcdir)/digikam/digikam \
#	       $(all_includes)
#
#digikaminclude_HEADERS = greycstorationiface.h greycstorationwidget.h greycstorationsettings.h 
#
#digikamincludedir = $(includedir)/digikam
#
