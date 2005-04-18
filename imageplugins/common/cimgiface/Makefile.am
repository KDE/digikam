METASOURCES = AUTO

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

noinst_LTLIBRARIES = libcimgiface.la

libcimgiface_la_SOURCES = cimgiface.cpp 

libcimgiface_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = $(all_includes) 

# Don't use CImg interface (keyboard/mouse interaction)
AM_CPPFLAGS = -Dcimg_OS=-1 -Dcimg_display_type=0 
