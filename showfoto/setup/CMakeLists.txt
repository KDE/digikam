
include_directories( ${CMAKE_SOURCE_DIR}/digikam/utilities/setup ${CMAKE_SOURCE_DIR}/digikam/digikam ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )






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
