METASOURCES = AUTO
SUBDIRS = patterns

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)   

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_border.la 

digikamimageplugin_border_la_SOURCES = imageplugin_border.cpp \
	                               imageeffect_border.cpp 

digikamimageplugin_border_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS)

digikamimageplugin_border_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_border.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_border.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_border_ui.rc

digikamimageplugin_bordericondir = $(kde_datadir)/digikam/icons
digikamimageplugin_bordericon_ICON = AUTO