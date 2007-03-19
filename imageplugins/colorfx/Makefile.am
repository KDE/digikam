METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/digikamimageplugins/common/include \
	       -I$(top_srcdir)/digikamimageplugins/common/widgets \
	       $(all_includes) $(LIBDIGIKAM_CFLAGS)   

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_colorfx.la

digikamimageplugin_colorfx_la_SOURCES = imageplugin_colorfx.cpp \
	                                     imageeffect_colorfx.cpp 

digikamimageplugin_colorfx_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS) \
	              $(top_builddir)/digikamimageplugins/common/widgets/libdigikamimagepluginswidget.la 

digikamimageplugin_colorfx_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_colorfx.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_colorfx.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_colorfx_ui.rc
       
digikamimageplugin_solarizeicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_solarizeicon_ICON = AUTO
