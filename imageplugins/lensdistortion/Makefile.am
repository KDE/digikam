METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS) 
        
# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_lensdistortion.la 

digikamimageplugin_lensdistortion_la_SOURCES = imageplugin_lensdistortion.cpp \
	                                       imageeffect_lensdistortion.cpp 

digikamimageplugin_lensdistortion_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS) 

digikamimageplugin_lensdistortion_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_lensdistortion.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_lensdistortion.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_lensdistortion_ui.rc
       
digikamimageplugin_lensdistortionicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_lensdistortionicon_ICON = AUTO
