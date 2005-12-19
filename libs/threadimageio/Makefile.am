METASOURCES = AUTO

noinst_LTLIBRARIES = libthreadimageio.la

libthreadimageio_la_SOURCES = loadsavethread.cpp

libthreadimageio_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
	       $(LIBKEXIF_CFLAGS) $(all_includes)
