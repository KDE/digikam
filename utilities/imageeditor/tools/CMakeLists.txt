
include_directories( ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/greycstoration ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############


########### install files ###############




#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libdimgeditortools.la 
#
#libdimgeditortools_la_SOURCES = imageresize.cpp imageprint.cpp
#
#libdimgeditortools_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_KDEPRINT) 
#
#INCLUDES= -I$(top_srcdir)/digikam/digikam \
#	  -I$(top_srcdir)/digikam/libs/dimg \
#	  -I$(top_srcdir)/digikam/libs/dimg/filters \
#	  -I$(top_srcdir)/digikam/libs/dmetadata \
#	  -I$(top_srcdir)/digikam/libs/greycstoration \
#	  -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
#	  -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
#	  $(all_includes) 
#
