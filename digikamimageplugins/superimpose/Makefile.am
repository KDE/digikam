METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)   

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_superimpose.la

digikamimageplugin_superimpose_la_SOURCES = imageplugin_superimpose.cpp dirselectwidget.cpp \
	                                    imageeffect_superimpose.cpp superimposewidget.cpp 

digikamimageplugin_superimpose_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS)

digikamimageplugin_superimpose_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_superimpose.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_superimpose.pot

KDE_OPTIONS=nofinal

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_superimpose_ui.rc

digikamimageplugin_superimposeicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_superimposeicon_ICON = AUTO
