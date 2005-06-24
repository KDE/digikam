METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)   

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_inserttext.la 

digikamimageplugin_inserttext_la_SOURCES = imageplugin_inserttext.cpp \
	                                   imageeffect_inserttext.cpp \
					   inserttextwidget.cpp 

digikamimageplugin_inserttext_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS)

digikamimageplugin_inserttext_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_inserttext.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_inserttext.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_inserttext_ui.rc

digikamimageplugin_inserttexticondir = $(kde_datadir)/digikam/icons
digikamimageplugin_inserttexticon_ICON = AUTO