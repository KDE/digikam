SUBDIRS = pics

lib_LTLIBRARIES = libdigikam.la
bin_PROGRAMS = digikam

INCLUDES = \
	-I$(top_srcdir)/digikam/utilities/kexif \
	-I$(top_srcdir)/digikam/utilities/imageviewer \
	-I$(top_srcdir)/digikam/utilities/splashscreen \
	-I$(top_srcdir)/digikam/utilities/setup \
	-I$(top_srcdir)/digikam/interfaces \
	$(all_includes)

libdigikam_la_SOURCES = digikamapp.cpp digikamview.cpp \
		albumfolderview.cpp albumfolderitem.cpp \
		albumiconview.cpp albumiconitem.cpp \
		thumbview.cpp thumbitem.cpp \
		albumpropsedit.cpp imagedescedit.cpp \
		albumsettings.cpp \
		cameralist.cpp cameratype.cpp cameradragobject.cpp \
		digikampluginmanager.cpp digikamcameraprocess.cpp \
		digikamfirstrun.cpp

libdigikam_la_LIBADD  = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI)  \
		$(LIBSOCKET) $(LIB_KFILE) \
		$(top_builddir)/digikam/utilities/kexif/libkexif.la \
		$(top_builddir)/digikam/utilities/imageviewer/libimageviewer.la \
		$(top_builddir)/digikam/utilities/splashscreen/libsplashscreen.la \
		$(top_builddir)/digikam/utilities/setup/libsetup.la \
		$(top_builddir)/digikam/interfaces/libdigikaminterfaces.la

libdigikam_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined


digikam_SOURCES = main.cpp
digikam_LDADD   = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI)  \
		$(LIBSOCKET) $(LIB_KFILE) $(LIB_KPARTS) \
		libdigikam.la
digikam_LDFLAGS = $(KDE_RPATH) $(all_libraries)

dummy_digikam.cpp:
	echo > dummy_digikam.cpp

METASOURCES = AUTO  

# this is where the kdelnk file will go 
kdelnkdir   = $(kde_appsdir)/Graphics
kdelnk_DATA = digikam.desktop


# Uncomment the following two lines if you add a ui.rc file for your application to make use of
# KDE2 XML GUI builing
rcdir = $(kde_datadir)/digikam
rc_DATA = digikamui.rc

