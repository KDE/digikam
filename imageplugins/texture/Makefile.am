METASOURCES = AUTO
SUBDIRS = patterns

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	       -I$(top_srcdir)/digikamimageplugins/common/widgets \
	       $(all_includes) $(LIBDIGIKAM_CFLAGS)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_texture.la

digikamimageplugin_texture_la_SOURCES = imageplugin_texture.cpp \
	                                imageeffect_texture.cpp texture.cpp 

digikamimageplugin_texture_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS)\
	              $(top_builddir)/digikamimageplugins/common/widgets/libdigikamimagepluginswidget.la 
digikamimageplugin_texture_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_texture.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_texture.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_texture_ui.rc        

digikamimageplugin_textureicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_textureicon_ICON = AUTO
