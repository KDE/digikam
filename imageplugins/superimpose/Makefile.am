METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/ \
	      $(all_includes) $(IMLIB2_CFLAGS) $(LIBDIGIKAM_CFLAGS)   

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_superimpose.la

digikamimageplugin_superimpose_la_SOURCES = \
	imageplugin_superimpose.cpp thumbbar.cpp \
	imageeffect_superimpose.cpp superimposewidget.cpp \
	dirselectwidget.cpp         

digikamimageplugin_superimpose_la_LIBADD = $(LIB_KPARTS) $(LIB_IMLIB2) $(LIBDIGIKAM_LIBS)

digikamimageplugin_superimpose_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_superimpose.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_superimpose.pot

KDE_OPTIONS=nofinal
