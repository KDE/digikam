bin_PROGRAMS = showfoto

INCLUDES = -I$(top_srcdir)/digikam/utilities/imageeditor \
	 $(LIBKEXIF_CFLAGS) $(all_includes)


showfoto_SOURCES = main.cpp showfoto.cpp thumbbar.cpp

showfoto_LDADD   = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) \
		   $(LIB_KFILE) $(LIB_KPARTS) $(LIBJPEG) \
		   $(top_builddir)/digikam/utilities/imageeditor/libimagecanvas.la \
		   $(top_builddir)/digikam/libs/histogram/libhistogram.la

showfoto_LDFLAGS = $(KDE_RPATH) $(all_libraries)

METASOURCES = AUTO  

rcdir = $(kde_datadir)/showfoto
rc_DATA = showfotoui.rc

xdg_apps_DATA = showfoto.desktop



