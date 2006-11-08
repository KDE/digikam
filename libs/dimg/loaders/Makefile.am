METASOURCES = AUTO
KDE_CXXFLAGS = $(USE_EXCEPTIONS)

noinst_LTLIBRARIES = libdimgloaders.la

libdimgloaders_la_SOURCES = dimgloader.cpp pngloader.cpp jpegloader.cpp tiffloader.cpp \
	                        rawloader.cpp ppmloader.cpp qimageloader.cpp iccjpeg.c 

libdimgloaders_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) \
	   	                    $(LIBJPEG) $(LIB_TIFF) $(LIB_PNG) 

INCLUDES = $(all_includes) -I$(top_srcdir)/digikam/libs/dimg \
	                       -I$(top_srcdir)/digikam/libs/dmetadata \
	                       -I$(top_srcdir)/digikam/libs/dcraw \
	                       -I$(top_srcdir)/digikam/digikam \
	                       -I$(top_srcdir)/digikam 	                   
