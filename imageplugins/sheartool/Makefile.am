METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/ \
	      $(all_includes) $(IMLIB2_CFLAGS) $(LIBDIGIKAM_CFLAGS)    
        
# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_sheartool.la

digikamimageplugin_sheartool_la_SOURCES = \
	imageplugin_sheartool.cpp \
	imageeffect_sheartool.cpp  

digikamimageplugin_sheartool_la_LIBADD = $(LIB_KPARTS) $(LIB_IMLIB2) $(LIBDIGIKAM_LIBS) 

digikamimageplugin_sheartool_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_sheartool.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_sheartool.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_sheartool_ui.rc
