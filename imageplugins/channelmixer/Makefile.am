METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      -I$(top_srcdir)/digikamimageplugins/common/dialogs \
	      -I$(top_srcdir)/digikamimageplugins/common/widgets \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_channelmixer.la

digikamimageplugin_channelmixer_la_SOURCES = imageplugin_channelmixer.cpp \
	                                     channelmixer.cpp 

digikamimageplugin_channelmixer_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS)\
	              $(top_builddir)/digikamimageplugins/common/dialogs/libdigikamimagepluginsdialog.la 

digikamimageplugin_channelmixer_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_channelmixer.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_channelmixer.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_channelmixer_ui.rc

digikamimageplugin_channelmixericondir = $(kde_datadir)/digikam/icons
digikamimageplugin_channelmixericon_ICON = AUTO
