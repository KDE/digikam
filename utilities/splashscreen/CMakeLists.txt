INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(splashscreen_STAT_SRCS
   splashscreen.cpp
   )

KDE3_AUTOMOC(${splashscreen_STAT_SRCS})

ADD_LIBRARY(splashscreen STATIC ${splashscreen_STAT_SRCS})


########### install files ###############

INSTALL_FILES( /share/apps/digikam FILES  digikam-splash.png )



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libsplashscreen.la
#
#libsplashscreen_la_SOURCES = splashscreen.cpp
#
#libsplashscreen_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#digikamsplashpicdir = $(kde_datadir)/digikam
#digikamsplashpic_DATA = digikam-splash.png
#
#INCLUDES= -I$(top_srcdir)/digikam/digikam \
#	  $(all_includes)
