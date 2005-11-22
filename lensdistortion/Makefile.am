METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
              -I$(top_srcdir)/digikam/digikam \
	      -I$(top_srcdir)/digikamimageplugins/common/dialogs \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS) 
        
# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_lensdistortion.la 

digikamimageplugin_lensdistortion_la_SOURCES = imageplugin_lensdistortion.cpp \
	                                       imageeffect_lensdistortion.cpp \
	                                       lensdistortion.cpp pixelaccess.cpp 

digikamimageplugin_lensdistortion_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS) \
	              $(top_builddir)/digikamimageplugins/common/dialogs/libdigikamimagepluginsdialog.la 

digikamimageplugin_lensdistortion_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_lensdistortion.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_lensdistortion.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_lensdistortion_ui.rc
       
digikamimageplugin_lensdistortionicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_lensdistortionicon_ICON = AUTO
