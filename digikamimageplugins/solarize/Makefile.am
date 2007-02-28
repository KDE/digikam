METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      -I$(top_srcdir)/digikamimageplugins/common/widgets \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)   

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_solarize.la

digikamimageplugin_solarize_la_SOURCES = imageplugin_solarize.cpp \
	                                 imageeffect_solarize.cpp 

digikamimageplugin_solarize_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS) \
	              $(top_builddir)/digikamimageplugins/common/widgets/libdigikamimagepluginswidget.la 

digikamimageplugin_solarize_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_solarize.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_solarize.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_solarize_ui.rc
       
digikamimageplugin_solarizeicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_solarizeicon_ICON = AUTO
