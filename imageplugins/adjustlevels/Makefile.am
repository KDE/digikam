METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      -I$(top_srcdir)/digikamimageplugins/common/dialogs \
	      -I$(top_srcdir)/digikamimageplugins/common/widgets \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_adjustlevels.la

digikamimageplugin_adjustlevels_la_SOURCES = imageplugin_adjustlevels.cpp \
	                                     adjustlevels.cpp 

digikamimageplugin_adjustlevels_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS) \
	              $(top_builddir)/digikamimageplugins/common/dialogs/libdigikamimagepluginsdialog.la 

digikamimageplugin_adjustlevels_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_adjustlevels.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_adjustlevels.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_adjustlevels_ui.rc

digikamimageplugin_adjustlevelsicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_adjustlevelsicon_ICON = AUTO
