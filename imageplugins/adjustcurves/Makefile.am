METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/ \
	      $(all_includes) $(IMLIB2_CFLAGS) 
                
# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_adjustcurves.la

digikamimageplugin_adjustcurves_la_SOURCES = \
	imageplugin_adjustcurves.cpp \
	adjustcurves.cpp 

digikamimageplugin_adjustcurves_la_LIBADD = $(LIB_KPARTS) -ldigikam 

digikamimageplugin_adjustcurves_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_adjustcurves.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_adjustcurves.pot

