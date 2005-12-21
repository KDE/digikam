METASOURCES = AUTO
SUBDIRS = pics

INCLUDES = -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
	   -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
	   -I$(top_srcdir)/digikam/libs/histogram \
	   -I$(top_srcdir)/digikam/libs/levels \
	   -I$(top_srcdir)/digikam/libs/filters \
	   -I$(top_srcdir)/digikam/libs/widgets \
	   -I$(top_srcdir)/digikam/libs/dialogs \
	   -I$(top_srcdir)/digikam/libs/dimg \
	   -I$(top_srcdir)/digikam/libs/dimg/filters \
	   -I$(top_srcdir)/digikam/digikam \
	   $(all_includes) $(IMLIB2_CFLAGS)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_core.la

digikamimageplugin_core_la_SOURCES = imageplugin_core.cpp \
		imageeffect_autocorrection.cpp imageeffect_bwsepia.cpp imageeffect_ratiocrop.cpp imageeffect_bcg.cpp \
		imageeffect_redeye.cpp imageeffect_blur.cpp imageeffect_sharpen.cpp imageeffect_rgb.cpp \
		imageeffect_hsl.cpp imageeffect_iccproof.cpp

digikamimageplugin_core_la_LIBADD = $(LIB_KPARTS) $(LIB_IMLIB2) \
	                                $(top_builddir)/digikam/digikam/libdigikam.la

digikamimageplugin_core_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

kde_services_DATA = digikamimageplugin_core.desktop

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_core_ui.rc

digikamimageplugin_coreicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_coreicon_ICON = AUTO
noinst_HEADERS = imageeffect_iccproof.h imageeffect_iccproof.h
