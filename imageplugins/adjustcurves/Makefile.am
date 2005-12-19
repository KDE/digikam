METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      -I$(top_srcdir)/digikamimageplugins/common/dialogs \
	      -I$(top_srcdir)/digikamimageplugins/common/widgets \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_adjustcurves.la

digikamimageplugin_adjustcurves_la_SOURCES = imageplugin_adjustcurves.cpp \
	                                     adjustcurves.cpp 

digikamimageplugin_adjustcurves_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS) \
	              $(top_builddir)/digikamimageplugins/common/dialogs/libdigikamimagepluginsdialog.la 

digikamimageplugin_adjustcurves_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
	              
kde_services_DATA = digikamimageplugin_adjustcurves.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_adjustcurves.pot

KDE_OPTIONS=nofinal

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_adjustcurves_ui.rc

digikamimageplugin_adjustcurvesicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_adjustcurvesicon_ICON = AUTO
