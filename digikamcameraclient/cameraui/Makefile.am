SUBDIRS = icons
METASOURCES = AUTO

noinst_LTLIBRARIES = libcameraui.la

libcameraui_la_SOURCES = cameraui.cpp camerafolderitem.cpp \
	cameraiconview.cpp gpcontroller.cpp gpmessages.cpp \
	camerafolderview.cpp camerainfodialog.cpp camerauiview.cpp \
	gpeventfilter.cpp gpstatus.cpp \
	cameraiconitem.cpp gpcamera.cpp \
	gpfileiteminfo.cpp gpfileiteminfodlg.cpp \
	savefiledialog.cpp camerasettings.cpp  \
	gpfileitemcontainer.cpp dmessagebox.cpp


libcameraui_la_LDFLAGS = $(LIB_GPHOTO) $(all_libraries) $(KDE_RPATH)

INCLUDES= \
	-I$(top_srcdir)/digikam/digikam \
	-I$(top_srcdir)/digikam/utilities/kexif \
	-I$(top_srcdir)/digikam/utilities/imageviewer \
	$(GPHOTO_CFLAGS) $(all_includes)


rcdir = $(kde_datadir)/digikamcameraclient
rc_DATA = digikamcameraui.rc
