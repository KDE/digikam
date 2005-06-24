METASOURCES = AUTO

noinst_LTLIBRARIES = libjpegutils.la

libjpegutils_la_SOURCES = jpegmetadata.cpp exifrotate.cpp transupp.cpp

libjpegutils_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
libjpegutils_la_LIBADD  = $(LIBJPEG) $(LIBKEXIF_LIBS)

INCLUDES = $(LIBKEXIF_CFLAGS) $(all_includes) 
