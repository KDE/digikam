METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      -I$(top_srcdir)/digikamimageplugins/common/dialogs \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS) 
        
# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_hotpixels.la 

digikamimageplugin_hotpixels_la_SOURCES = blackframeparser.cpp weights.cpp hotpixelfixer.cpp imageplugin_hotpixels.cpp \
					  blackframelistview.h blackframelistview.cpp imageeffect_hotpixels.cpp 

digikamimageplugin_hotpixels_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS) \
	              $(top_builddir)/digikamimageplugins/common/dialogs/libdigikamimagepluginsdialog.la 

digikamimageplugin_hotpixels_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_hotpixels.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_hotpixels.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_hotpixels_ui.rc
       
digikamimageplugin_hotpixelsicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_hotpixelsicon_ICON = AUTO
