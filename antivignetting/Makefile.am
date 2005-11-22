METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
              -I$(top_srcdir)/digikam/digikam \
	      -I$(top_srcdir)/digikamimageplugins/common/dialogs \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)   

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_antivignetting.la 

digikamimageplugin_antivignetting_la_SOURCES = imageplugin_antivignetting.cpp \
	                                       imageeffect_antivignetting.cpp \
					       antivignetting.cpp 

digikamimageplugin_antivignetting_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS) \
	              $(top_builddir)/digikamimageplugins/common/dialogs/libdigikamimagepluginsdialog.la 

digikamimageplugin_antivignetting_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_antivignetting.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_antivignetting.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_antivignetting_ui.rc

digikamimageplugin_antivignettingicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_antivignettingicon_ICON = AUTO
