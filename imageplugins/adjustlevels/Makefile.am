METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikam/utilities/imageeditor \
	-I$(top_srcdir)/digikam/libs/guibuilder \
	-I$(top_srcdir)/digikam/libs/histogram \
	-I$(top_srcdir)/digikam/libs/levels \
	$(all_includes)   
        
# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_adjustlevels.la

digikamimageplugin_adjustlevels_la_SOURCES = \
	imageplugin_adjustlevels.cpp \
	adjustlevels.cpp

digikamimageplugin_adjustlevels_la_LIBADD = $(LIB_KPARTS) \
	$(top_builddir)/digikam/digikam/libdigikam.la

digikamimageplugin_adjustlevels_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

kde_services_DATA = digikamimageplugin_adjustlevels.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_adjustlevels.pot

