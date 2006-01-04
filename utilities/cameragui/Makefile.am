METASOURCES = AUTO

noinst_LTLIBRARIES = libcameragui.la

libcameragui_la_SOURCES = cameraui.cpp cameraiconview.cpp  \
			  cameraiconitem.cpp cameracontroller.cpp \
			  animwidget.cpp renamecustomizer.cpp  \
			  dkcamera.cpp gpcamera.cpp umscamera.cpp \
			  gpiteminfo.cpp gpiteminfodlg.cpp camerainfodialog.cpp \
			  albumselectdialog.cpp

libcameragui_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
libcameragui_la_LIBADD  = $(LIB_GPHOTO) $(LIBKEXIF_LIBS) $(LIBJPEG)

INCLUDES= -I$(top_srcdir)/digikam/digikam \
	      -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
	      -I$(top_srcdir)/digikam/libs/jpegutils \
	      -I$(top_srcdir)/digikam/libs/dimg \
	      -I$(top_srcdir)/digikam/libs/dcraw \
	      $(GPHOTO_CFLAGS) $(LIBKEXIF_CFLAGS) $(all_includes)

digikamcameraguiicondir = $(kde_datadir)/digikam/icons
digikamcameraguiicon_ICON = AUTO

KDE_OPTIONS=nofinal
