METASOURCES = AUTO

# --enable-final triggers: http://bugs.kde.org/show_bug.cgi?id=126326
# digikam: camera download: auto-rotated images lose EXIF info ...
# So make sure nofinal is always used here!
KDE_OPTIONS = nofinal

INCLUDES = $(all_includes) \
	   -I$(top_srcdir)/digikam/libs/dmetadata \
	   -I$(top_srcdir)/digikam/libs/dimg \
	   -I$(top_srcdir)/digikam/digikam \
	   $(LIBKDCRAW_CFLAGS) \
	   $(LIBKEXIV2_CFLAGS) 


noinst_LTLIBRARIES = libjpegutils.la

libjpegutils_la_SOURCES = jpegutils.cpp transupp.cpp

libjpegutils_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

libjpegutils_la_LIBADD  = $(LIBJPEG) 
