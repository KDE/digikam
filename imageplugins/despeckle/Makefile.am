METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikam/utilities/imageeditor \
	-I$(top_srcdir)/digikam/libs/guibuilder \
	-I$(top_srcdir)/digikam/libs/widgets \
	$(all_includes)   
        
# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_despeckle.la

digikamimageplugin_despeckle_la_SOURCES = \
	imageplugin_despeckle.cpp \
	despeckle.cpp

digikamimageplugin_despeckle_la_LIBADD = $(LIB_KPARTS) \
	$(top_builddir)/digikam/digikam/libdigikam.la

digikamimageplugin_despeckle_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

kde_services_DATA = digikamimageplugin_despeckle.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_despeckle.pot

