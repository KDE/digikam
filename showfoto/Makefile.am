SUBDIRS = pics 

METASOURCES = AUTO  

bin_PROGRAMS = showfoto

INCLUDES = -I$(top_srcdir)/digikam/utilities/imageeditor \
	   -I$(top_srcdir)/digikam/libs/thumbbar/ \
	   -I$(top_srcdir)/digikam/libs/widgets \
	   -I$(top_srcdir)/digikam/libs/histogram \
	   -I$(top_srcdir)/digikam/libs/dcraw \
	   -I$(top_srcdir)/digikam/digikam \
	   -I$(top_srcdir)/digikam/utilities/splashscreen \
	   $(LIBKEXIF_CFLAGS) $(all_includes)

showfoto_SOURCES = main.cpp showfoto.cpp setup.cpp slideshow.cpp \
		   setupeditor.cpp setupplugins.cpp setupslideshow.cpp \
		   imageproperties.cpp imagepropertiesgeneral.cpp \
		   imagepropertiesexif.cpp imagepropertieshistogram.cpp 

showfoto_LDADD   = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KUTILS) \
		   $(LIB_KFILE) $(LIB_KPARTS) $(LIBJPEG) \
		   $(top_builddir)/digikam/libs/histogram/libhistogram.la \
		   $(top_builddir)/digikam/libs/widgets/libwidgets.la \
		   $(top_builddir)/digikam/libs/thumbbar/libthumbbar.la \
		   $(top_builddir)/digikam/libs/curves/libcurves.la \
		   $(top_builddir)/digikam/utilities/imageeditor/libimageeditor.la \
		   $(top_builddir)/digikam/utilities/imageeditor/libimagecanvas.la \
		   $(top_builddir)/digikam/utilities/imageeditor/libshowfoto.la \
		   $(top_builddir)/digikam/utilities/splashscreen/libsplashscreen.la                    

showfoto_LDFLAGS = $(KDE_RPATH) $(all_libraries)

rcdir = $(kde_datadir)/showfoto
rc_DATA = showfotoui.rc

xdg_apps_DATA = showfoto.desktop


