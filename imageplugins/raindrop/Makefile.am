METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      -I$(top_srcdir)/digikamimageplugins/common/dialogs \
		  -I$(top_srcdir)/digikam/digikam \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)   

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_raindrop.la 

digikamimageplugin_raindrop_la_SOURCES = imageplugin_raindrop.cpp \
	                                 imageeffect_raindrop.cpp raindrop.cpp 

digikamimageplugin_raindrop_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS)\
	              $(top_builddir)/digikamimageplugins/common/dialogs/libdigikamimagepluginsdialog.la 

digikamimageplugin_raindrop_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_raindrop.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_raindrop.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_raindrop_ui.rc     
       
digikamimageplugin_raindropicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_raindropicon_ICON = AUTO
