SUBDIRS = pics icons 

METASOURCES = AUTO  

bin_PROGRAMS = showfoto

INCLUDES = -I$(top_srcdir)/digikam/libs/thumbbar \
	   -I$(top_srcdir)/digikam/libs/widgets \
	   -I$(top_srcdir)/digikam/libs/imageproperties \
	   -I$(top_srcdir)/digikam/libs/histogram \
   	   -I$(top_srcdir)/digikam/libs/dcraw \
   	   -I$(top_srcdir)/digikam/libs/dimg \
	   -I$(top_srcdir)/digikam/digikam \
	   -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
	   -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
	   -I$(top_srcdir)/digikam/utilities/splashscreen \
	   $(LIBKEXIF_CFLAGS) $(all_includes)

showfoto_SOURCES = main.cpp showfoto.cpp setup.cpp slideshow.cpp \
		   setupeditor.cpp setupplugins.cpp setupslideshow.cpp \
		   setupicc.cpp 

showfoto_LDADD   = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KUTILS) \
		   $(LIB_KFILE) $(LIB_KPARTS) $(LIBJPEG) \
		   $(top_builddir)/digikam/libs/widgets/libwidgets.la \
		   $(top_builddir)/digikam/libs/imageproperties/libimagepropertiesshowfoto.la \
		   $(top_builddir)/digikam/libs/thumbbar/libthumbbar.la \
		   $(top_builddir)/digikam/utilities/imageeditor/canvas/libdimgcanvas.la \
           $(top_builddir)/digikam/utilities/imageeditor/editor/libdimgeditor.la \
		   $(top_builddir)/digikam/utilities/imageeditor/editor/libshowfoto.la \
		   $(top_builddir)/digikam/utilities/splashscreen/libsplashscreen.la 

showfoto_LDFLAGS = $(KDE_RPATH) $(all_libraries)

rcdir = $(kde_datadir)/showfoto
rc_DATA = showfotoui.rc

xdg_apps_DATA = showfoto.desktop
