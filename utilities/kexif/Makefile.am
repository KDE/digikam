METASOURCES = AUTO

noinst_LTLIBRARIES = libkexif.la

libkexif_la_SOURCES = kexif.cpp kexifentry.cpp kexifdata.cpp \
		kexififd.cpp kexiflistview.cpp kexiflistviewitem.cpp

libkexif_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIBEXIF_LIBS)

INCLUDES= $(all_includes) $(LIBEXIF_CFLAGS)
