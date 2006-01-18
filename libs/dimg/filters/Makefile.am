METASOURCES = AUTO

noinst_LTLIBRARIES = libdimgfilters.la

libdimgfilters_la_SOURCES = bcgmodifier.cpp hslmodifier.cpp icctransform.cpp \
	                        dimgimagefilters.cpp dimgthreadedfilter.cpp \
	                        dimggaussianblur.cpp dimgsharpen.cpp colormodifier.cpp 

libdimgfilters_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
	                       -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
	                       -I$(top_srcdir)/digikam/libs/levels \
	                       -I$(top_srcdir)/digikam/libs/histogram \
	                       -I$(top_srcdir)/digikam/digikam \
	                       $(all_includes)

digikaminclude_HEADERS = bcgmodifier.h hslmodifier.h dimgthreadedfilter.h dimgimagefilters.h icctransform.h colormodifier.h 
digikamincludedir = $(includedir)/digikam
