METASOURCES = AUTO
INCLUDES    = -I$(top_srcdir)/digikam/utilities/imageeditor \
	-I$(top_srcdir)/digikam/libs/guibuilder \
	-I$(top_srcdir)/digikam/libs/histogram \
	-I$(top_srcdir)/digikam/libs/levels \
	-I$(top_srcdir)/digikam/libs/widgets \
	$(all_includes) $(IMLIB2_CFLAGS)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_core.la

digikamimageplugin_core_la_SOURCES = \
	imageplugin_core.cpp \
	imageeffect_bwsepia.cpp \
	imageeffect_bcg.cpp imageeffect_redeye.cpp \
	imageeffect_blur.cpp imageeffect_sharpen.cpp \
	imageeffect_rgb.cpp imageeffect_hsl.cpp \
	imageeffect_colorsenhance.cpp 

digikamimageplugin_core_la_LIBADD = $(LIB_KPARTS) \
	$(top_builddir)/digikam/digikam/libdigikam.la

digikamimageplugin_core_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

kde_services_DATA = digikamimageplugin_core.desktop

