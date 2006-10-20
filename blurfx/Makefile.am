METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	       -I$(top_srcdir)/digikamimageplugins/common/widgets \
	       $(all_includes) $(LIBDIGIKAM_CFLAGS)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_blurfx.la 

digikamimageplugin_blurfx_la_SOURCES = imageplugin_blurfx.cpp \
	                               imageeffect_blurfx.cpp blurfx.cpp 

digikamimageplugin_blurfx_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS) \
	              $(top_builddir)/digikamimageplugins/common/widgets/libdigikamimagepluginswidget.la

digikamimageplugin_blurfx_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_blurfx.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_blurfx.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_blurfx_ui.rc     
       
digikamimageplugin_blurfxicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_blurfxicon_ICON = AUTO
