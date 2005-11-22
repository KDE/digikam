METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      -I$(top_srcdir)/digikamimageplugins/common/dialogs \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)    
        
# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_sheartool.la

digikamimageplugin_sheartool_la_SOURCES = imageplugin_sheartool.cpp \
	                                  imageeffect_sheartool.cpp sheartool.cpp 

digikamimageplugin_sheartool_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS) \
	              $(top_builddir)/digikamimageplugins/common/dialogs/libdigikamimagepluginsdialog.la 

digikamimageplugin_sheartool_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_sheartool.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_sheartool.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_sheartool_ui.rc

digikamimageplugin_sheartoolicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_sheartoolicon_ICON = AUTO
