METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)   

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_infrared.la

digikamimageplugin_infrared_la_SOURCES = imageplugin_infrared.cpp \
	                                 imageeffect_infrared.cpp 

digikamimageplugin_infrared_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS)

digikamimageplugin_infrared_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_infrared.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_infrared.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_infrared_ui.rc        

digikamimageplugin_infraredicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_infraredicon_ICON = AUTO
