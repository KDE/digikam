METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/ \
	      $(all_includes) $(IMLIB2_CFLAGS) $(LIBDIGIKAM_CFLAGS)   

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_border.la 

digikamimageplugin_border_la_SOURCES = \
	imageplugin_border.cpp \
	imageeffect_border.cpp 

digikamimageplugin_border_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS)

digikamimageplugin_border_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_border.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_border.pot

borderpicsdir = $(kde_datadir)/digikamimageplugins/data
borderpics_DATA = pine-pattern.png wood-pattern.png paper-pattern.png parque-pattern.png \
	          ice-pattern.png leaf-pattern.png marble-pattern.png rain-pattern.png \
	          craters-pattern.png dried-pattern.png pink-pattern.png stone-pattern.png \
	          chalk-pattern.png granit-pattern.png rock-pattern.png wall-pattern.png

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_border_ui.rc
