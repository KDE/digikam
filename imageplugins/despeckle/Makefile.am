METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_despeckle.la

digikamimageplugin_despeckle_la_SOURCES = imageplugin_despeckle.cpp \
	                                  despeckle.cpp 

digikamimageplugin_despeckle_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS)

digikamimageplugin_despeckle_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_despeckle.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_despeckle.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_despeckle_ui.rc
       
digikamimageplugin_despeckleicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_despeckleicon_ICON = AUTO
