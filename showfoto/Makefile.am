SUBDIRS = pics icons setup 

METASOURCES = AUTO  

bin_PROGRAMS = showfoto

INCLUDES = -I$(top_srcdir)/digikam/showfoto/setup \
	   -I$(top_srcdir)/digikam/utilities/setup \
	   -I$(top_srcdir)/digikam/libs/thumbbar \
	   -I$(top_srcdir)/digikam/libs/widgets/common \
	   -I$(top_srcdir)/digikam/libs/imageproperties \
	   -I$(top_srcdir)/digikam/libs/histogram \
	   -I$(top_srcdir)/digikam/libs/dcraw \
	   -I$(top_srcdir)/digikam/libs/dimg \
	   -I$(top_srcdir)/digikam/libs/dmetadata \
	   -I$(top_srcdir)/digikam/libs/threadimageio \
	   -I$(top_srcdir)/digikam/digikam \
	   -I$(top_srcdir)/digikam/utilities/slideshow \
	   -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
	   -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
	   -I$(top_srcdir)/digikam/utilities/imageeditor/tools \
	   -I$(top_srcdir)/digikam/utilities/splashscreen \
	   $(all_includes)

showfoto_SOURCES = main.cpp showfoto.cpp 

showfoto_LDADD   = $(top_builddir)/digikam/showfoto/setup/libsetup.la \
	           $(top_builddir)/digikam/libs/dialogs/libdialogshowfoto.la \
	           $(top_builddir)/digikam/libs/imageproperties/libimagepropertiesshowfoto.la \
	           $(top_builddir)/digikam/libs/thumbbar/libthumbbar.la \
	           $(top_builddir)/digikam/utilities/imageeditor/canvas/libdimgcanvas.la \
	           $(top_builddir)/digikam/utilities/imageeditor/editor/libshowfoto.la \
	           $(top_builddir)/digikam/utilities/splashscreen/libsplashscreen.la \
	           $(top_builddir)/digikam/utilities/slideshow/libslideshow.la \
	           $(top_builddir)/digikam/libs/threadimageio/libthreadimageio.la \
	           $(top_builddir)/digikam/libs/jpegutils/libjpegutils.la \
	           $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KUTILS) \
	           $(LIB_KFILE) $(LIB_KPARTS) $(LIBJPEG) $(EXIV2_LIBS) 

showfoto_LDFLAGS = $(KDE_RPATH) $(all_libraries)

rcdir = $(kde_datadir)/showfoto
rc_DATA = showfotoui.rc

xdg_apps_DATA = showfoto.desktop
