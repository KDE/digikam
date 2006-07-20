METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	       -I$(top_srcdir)/digikamimageplugins/common/widgets \
	       $(all_includes) $(LIBDIGIKAM_CFLAGS)
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)   

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_filmgrain.la

digikamimageplugin_filmgrain_la_SOURCES = imageplugin_filmgrain.cpp \
	                                  imageeffect_filmgrain.cpp filmgrain.cpp 

digikamimageplugin_filmgrain_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS)\
	              $(top_builddir)/digikamimageplugins/common/widgets/libdigikamimagepluginswidget.la 

digikamimageplugin_filmgrain_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_filmgrain.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_filmgrain.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_filmgrain_ui.rc        

digikamimageplugin_filmgrainicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_filmgrainicon_ICON = AUTO
