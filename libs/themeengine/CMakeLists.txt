
include_directories( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/ ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############


########### install files ###############




#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libthemeengine.la
#
#libthemeengine_la_SOURCES = theme.cpp themeengine.cpp texture.cpp
#
#libthemeengine_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
# 	       -I$(top_srcdir)/digikam/digikam \
#	       -I$(top_srcdir)/digikam/ \
#	       $(all_includes)
