METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/ \
	      $(all_includes) $(IMLIB2_CFLAGS) $(LIBDIGIKAM_CFLAGS)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_adjustlevels.la

digikamimageplugin_adjustlevels_la_SOURCES = \
	imageplugin_adjustlevels.cpp \
	adjustlevels.cpp 

digikamimageplugin_adjustlevels_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS)

digikamimageplugin_adjustlevels_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_adjustlevels.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_adjustlevels.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_adjustlevels_ui.rc
