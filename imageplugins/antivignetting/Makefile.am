METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/ \
	      $(all_includes) $(IMLIB2_CFLAGS)   
        
# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_antivignetting.la 

digikamimageplugin_antivignetting_la_SOURCES = \
	imageplugin_antivignetting.cpp \
	imageeffect_antivignetting.cpp 

digikamimageplugin_antivignetting_la_LIBADD = $(LIB_KPARTS) -ldigikam 

digikamimageplugin_antivignetting_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_antivignetting.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_antivignetting.pot

