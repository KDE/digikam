METASOURCES = AUTO

noinst_LTLIBRARIES = libimageviewer.la

libimageviewer_la_SOURCES = imageview.cpp canvas.cpp \
		imlibinterface.cpp exifrestorer.cpp \
		imeventfilter.cpp  imagebcgedit.cpp

libimageviewer_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_IMLIB)

INCLUDES= $(IMLIB_CFLAGS) $(all_includes) 
