METASOURCES = AUTO

noinst_LTLIBRARIES = libdimgloaders.la

libdimgloaders_la_SOURCES = dimgloader.cpp pngloader.cpp jpegloader.cpp tiffloader.cpp \
	                    rawloader.cpp ppmloader.cpp qimageloader.cpp iccjpeg.c \
	                    jp2kloader.cpp jpegsettings.cpp pngsettings.cpp \
	                    tiffsettings.cpp jp2ksettings.cpp 

libdimgloaders_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) \
	                    $(LIBJPEG) $(LIB_TIFF) $(LIB_PNG) $(LIB_JASPER)

INCLUDES = $(all_includes) -I$(top_srcdir)/digikam/libs/dimg \
	                   -I$(top_srcdir)/digikam/libs/dimg/filters \
	                   -I$(top_srcdir)/digikam/libs/curves \
	                   -I$(top_srcdir)/digikam/libs/levels \
	                   -I$(top_srcdir)/digikam/libs/histogram \
	                   -I$(top_srcdir)/digikam/libs/whitebalance \
	                   -I$(top_srcdir)/digikam/libs/dmetadata \
	                   -I$(top_srcdir)/digikam/digikam \
	                   $(LIBKEXIV2_CFLAGS) $(LIBKDCRAW_CFLAGS) 
