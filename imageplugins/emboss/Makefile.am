METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	       -I$(top_srcdir)/digikamimageplugins/common/widgets \
	       $(all_includes) $(LIBDIGIKAM_CFLAGS)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_emboss.la

digikamimageplugin_emboss_la_SOURCES = imageplugin_emboss.cpp \
	                               imageeffect_emboss.cpp emboss.cpp 

digikamimageplugin_emboss_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS) \
	              $(top_builddir)/digikamimageplugins/common/widgets/libdigikamimagepluginswidget.la

digikamimageplugin_emboss_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_emboss.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_emboss.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_emboss_ui.rc        

digikamimageplugin_embossicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_embossicon_ICON = AUTO
