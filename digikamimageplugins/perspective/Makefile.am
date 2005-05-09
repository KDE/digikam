METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)   

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_perspective.la

digikamimageplugin_perspective_la_SOURCES = imageplugin_perspective.cpp \
	                                    imageeffect_perspective.cpp \
					    perspectivewidget.cpp 

digikamimageplugin_perspective_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS)

digikamimageplugin_perspective_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_perspective.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_perspective.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_perspective_ui.rc

digikamimageplugin_perspectiveicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_perspectiveicon_ICON = AUTO
