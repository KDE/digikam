METASOURCES = AUTO

noinst_LTLIBRARIES = libcameragui.la

libcameragui_la_SOURCES = cameraui.cpp   cameraiconview.cpp  \
			cameraiconitem.cpp cameracontroller.cpp \
			dirselectdialog.cpp animwidget.cpp \
			renamecustomizer.cpp gpcamera.cpp gpiteminfo.cpp \
			camerapropsplugin.cpp camerainfodialog.cpp \
			exifrotate.cpp transupp.cpp

libcameragui_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
libcameragui_la_LIBADD  = $(LIB_GPHOTO) -ljpeg

INCLUDES= -I$(top_srcdir)/digikam/digikam \
	-I$(top_srcdir)/digikam/utilities/imageeditor \
	$(all_includes) $(GPHOTO_CFLAGS)

digikamcameraguiicondir = $(kde_datadir)/digikam/icons
digikamcameraguiicon_ICON = AUTO

