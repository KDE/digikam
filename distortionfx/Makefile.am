METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)   

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_distortionfx.la 

digikamimageplugin_distortionfx_la_SOURCES = imageplugin_distortionfx.cpp \
	                                 imageeffect_distortionfx.cpp 

digikamimageplugin_distortionfx_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS)

digikamimageplugin_distortionfx_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_distortionfx.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_distortionfx.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_distortionfx_ui.rc     
       
digikamimageplugin_distortionfxicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_distortionfxicon_ICON = AUTO
