METASOURCES = AUTO
INCLUDES    = -I$(top_srcdir)/digikam/utilities/imageeditor $(all_includes)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_core.la

digikamimageplugin_core_la_SOURCES = imageplugin_core.cpp \
	imageeffect_solarize.cpp imageeffect_bwsepia.cpp \
	imageeffect_bcg.cpp imageeffect_redeye.cpp

digikamimageplugin_core_la_LIBADD = $(LIB_KPARTS) \
	$(top_builddir)/digikam/digikam/libdigikam.la

digikamimageplugin_core_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

kde_services_DATA = digikamimageplugin_core.desktop

