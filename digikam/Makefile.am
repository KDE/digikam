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
		$(top_builddir)/utilities/kexif/libkexif.la \
		$(top_builddir)/utilities/imageviewer/libimageviewer.la \
		$(top_builddir)/utilities/splashscreen/libsplashscreen.la \
		$(top_builddir)/utilities/setup/libsetup.la \
		$(top_builddir)/interfaces/libdigikaminterfaces.la

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
# KDE´s XML GUI builing
rcdir = $(kde_datadir)/digikam
rc_DATA = digikamui.rc


#WARNING: if you use a ui.rc file above, use:

# messages: rc.cpp

# instead of 

# messages:

# messages: rc.cpp
# 	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
# 	if test -n "$$LIST"; then \
# 		$(XGETTEXT) $$LIST -o $(podir)/digikam.pot; \
# 	fi
# These paths are KDE specific. Use them:
# kde_appsdir         Where your application's menu entry (.desktop) should go to.
# kde_icondir         Where your icon should go to - better use KDE_ICON.
# kde_sounddir        Where your sounds should go to.
# kde_htmldir         Where your docs should go to. (contains lang subdirs)
# kde_datadir         Where you install application data. (Use a subdir)
# kde_locale          Where translation files should go to. (contains lang subdirs)
# kde_cgidir          Where cgi-bin executables should go to.
# kde_confdir         Where config files should go to (system-wide ones with default values).
# kde_mimedir         Where mimetypes .desktop files should go to.
# kde_servicesdir     Where services .desktop files should go to.
# kde_servicetypesdir Where servicetypes .desktop files should go to.
# kde_toolbardir      Where general toolbar icons should go to (deprecated, use KDE_ICON).
# kde_wallpaperdir    Where general wallpapers should go to.
# kde_templatesdir    Where templates for the "New" menu (Konqueror/KDesktop) should go to.
# kde_bindir          Where executables should go to. Use bin_PROGRAMS or bin_SCRIPTS.
# kde_libdir          Where shared libraries should go to. Use lib_LTLIBRARIES.
# kde_moduledir       Where modules (e.g. parts) should go to. Use kde_module_LTLIBRARIES.
# kde_styledir        Where Qt/KDE widget styles should go to (new in KDE 3).
# kde_designerdir     Where Qt Designer plugins should go to (new in KDE 3).

