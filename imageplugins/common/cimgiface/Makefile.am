METASOURCES = AUTO

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

noinst_LTLIBRARIES = libcimgiface.la

libcimgiface_la_SOURCES = cimgiface.cpp 

libcimgiface_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = $(all_includes) 

# Common arguments for optimizations
AM_CXXFLAGS = -Wall -O3 -ffast-math

# Don't use CImg interface (keyboard/mouse interaction)
AM_CXXFLAGS += -Dcimg_OS=-1 -Dcimg_display_type=0 
