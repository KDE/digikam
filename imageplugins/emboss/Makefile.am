METASOURCES = AUTO

INCLUDES    = $(all_includes) 
        
# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_emboss.la

digikamimageplugin_emboss_la_SOURCES = \
	imageplugin_emboss.cpp \
	imageeffect_emboss.cpp 

digikamimageplugin_emboss_la_LIBADD = $(LIB_KPARTS) -ldigikam 

digikamimageplugin_emboss_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_emboss.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_emboss.pot

