METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)   

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_whitebalance.la

digikamimageplugin_whitebalance_la_SOURCES = imageplugin_whitebalance.cpp \
	                                     imageeffect_whitebalance.cpp 

digikamimageplugin_whitebalance_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS)

digikamimageplugin_whitebalance_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_whitebalance.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_whitebalance.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_whitebalance_ui.rc

digikamimageplugin_whitebalanceicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_whitebalanceicon_ICON = AUTO
