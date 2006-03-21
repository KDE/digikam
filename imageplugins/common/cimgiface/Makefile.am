METASOURCES = AUTO

KDE_CXXFLAGS = $(USE_EXCEPTIONS) -w 

noinst_LTLIBRARIES = libcimgiface.la

libcimgiface_la_SOURCES = cimgiface.cpp 

libcimgiface_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = -I$(top_srcdir)/digikamimageplugins/common/include \
           -I$(top_srcdir)/digikam/digikam \
	       $(all_includes) $(LIBDIGIKAM_CFLAGS) 

# Don't use CImg interface (keyboard/mouse interaction)
AM_CPPFLAGS = -Dcimg_OS=-1 -Dcimg_display_type=0 
