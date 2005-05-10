bin_PROGRAMS = showfoto

INCLUDES = -I$(top_srcdir)/digikam/utilities/imageeditor \
	   -I$(top_srcdir)/digikam/libs/thumbbar/ \
	   -I$(top_srcdir)/digikam/libs/widgets \
	   -I$(top_srcdir)/digikam/libs/histogram \
	   -I$(top_srcdir)/digikam/digikam \
	   -I$(top_srcdir)/digikam/utilities/splashscreen \
	   $(LIBKEXIF_CFLAGS) $(all_includes)

showfoto_SOURCES = main.cpp showfoto.cpp setup.cpp setupeditor.cpp \
	           setupplugins.cpp imageproperties.cpp imagepropertiesgeneral.cpp \
		   imagepropertiesexif.cpp imagepropertieshistogram.cpp 

showfoto_LDADD   = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) \
		   $(LIB_KFILE) $(LIB_KPARTS) $(LIBJPEG) \
		   $(top_builddir)/digikam/utilities/imageeditor/libimagecanvas.la \
		   $(top_builddir)/digikam/utilities/imageeditor/libshowfoto.la \
		   $(top_builddir)/digikam/libs/histogram/libhistogram.la \
		   $(top_builddir)/digikam/libs/widgets/libwidgets.la \
		   $(top_builddir)/digikam/utilities/splashscreen/libsplashscreen.la \
		   $(top_builddir)/digikam/libs/thumbbar/libthumbbar.la 

showfoto_LDFLAGS = $(KDE_RPATH) $(all_libraries)

METASOURCES = AUTO  

rcdir = $(kde_datadir)/showfoto
rc_DATA = showfotoui.rc

showfotosplashpicdir = $(kde_datadir)/showfoto
showfotosplashpic_DATA = showfoto-splash.png

xdg_apps_DATA = showfoto.desktop

KDE_ICON = showfoto 



