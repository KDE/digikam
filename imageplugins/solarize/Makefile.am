METASOURCES = AUTO

INCLUDES    = $(all_includes) 
        
# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_solarize.la

digikamimageplugin_solarize_la_SOURCES = \
	imageplugin_solarize.cpp \
	imageeffect_solarize.cpp 

digikamimageplugin_solarize_la_LIBADD = $(LIB_KPARTS) -ldigikam

digikamimageplugin_solarize_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_solarize.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_solarize.pot

