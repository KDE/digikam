INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/utilities/setup ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(setup_STAT_SRCS
   setup.cpp
   setupeditor.cpp
   setuptooltip.cpp
   )

KDE3_AUTOMOC(${setup_STAT_SRCS})

ADD_LIBRARY(setup STATIC ${setup_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#INCLUDES = -I$(top_srcdir)/digikam/utilities/setup \
#	   -I$(top_srcdir)/digikam/digikam \
#	   $(LIBKDCRAW_CFLAGS) $(all_includes)
#
#noinst_LTLIBRARIES = libsetup.la
#
#libsetup_la_SOURCES = setup.cpp setupeditor.cpp setuptooltip.cpp
#
#libsetup_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
#
#libsetup_la_LIBADD = $(top_builddir)/digikam/utilities/setup/libshowfotosetup.la 
#
