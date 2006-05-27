METASOURCES = AUTO
KDE_CXXFLAGS = $(USE_EXCEPTIONS)

noinst_LTLIBRARIES = libdmetadataloader.la

libdmetadataloader_la_SOURCES = dmetaloader.cpp jpegmetaloader.cpp pngmetaloader.cpp tiffmetaloader.cpp \
	                        rawmetaloader.cpp 

libdmetadataloader_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIBJPEG) $(LIB_TIFF) $(LIB_PNG)

INCLUDES = -I$(top_srcdir)/digikam/libs/dmetadata \
	   -I$(top_srcdir)/digikam/libs/dimg \
	   -I$(top_srcdir)/digikam/libs/dcraw \
	   -I$(top_srcdir)/digikam/digikam \
	   $(all_includes)
