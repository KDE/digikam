METASOURCES = AUTO

noinst_LTLIBRARIES = libcameragui.la

libcameragui_la_SOURCES = cameraui.cpp cameraiconview.cpp  \
	                  cameraiconitem.cpp cameracontroller.cpp \
	                  camerafolderview.cpp camerafolderitem.cpp \
	                  animwidget.cpp renamecustomizer.cpp  \
	                  dkcamera.cpp gpcamera.cpp umscamera.cpp \
	                  gpiteminfo.cpp camerainfodialog.cpp \
	                  albumselectdialog.cpp camerafolderdialog.cpp 

libcameragui_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

libcameragui_la_LIBADD  = $(top_builddir)/digikam/libs/imageproperties/libimagepropertiescamgui.la \
	                      $(LIB_GPHOTO) $(LIBJPEG)

INCLUDES= -I$(top_srcdir)/digikam/digikam \
	  -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
	  -I$(top_srcdir)/digikam/libs/jpegutils \
	  -I$(top_srcdir)/digikam/libs/themeengine \
	  -I$(top_srcdir)/digikam/libs/imageproperties \
	  -I$(top_srcdir)/digikam/libs/widgets/common \
	  -I$(top_srcdir)/digikam/libs/dimg \
	  -I$(top_srcdir)/digikam/libs/dmetadata \
	  -I$(top_srcdir)/digikam/libs/dcraw \
	  $(GPHOTO_CFLAGS) $(all_includes)

KDE_OPTIONS=nofinal
