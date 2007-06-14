
include_directories( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters ${CMAKE_SOURCE_DIR}/digikam/digikam ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############


########### install files ###############

install( FILES  greycstorationiface.h greycstorationwidget.h greycstorationsettings.h  DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )



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
