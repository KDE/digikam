METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/digikamimageplugins/common/include \
	       -I$(top_srcdir)/digikamimageplugins/common/widgets \
	       $(all_includes) $(LIBDIGIKAM_CFLAGS)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_superimpose.la

digikamimageplugin_superimpose_la_SOURCES = superimposewidget.cpp superimpose.cpp dirselectwidget.cpp \
	                                    imageplugin_superimpose.cpp imageeffect_superimpose.cpp 
	                                        

digikamimageplugin_superimpose_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS) \
	              $(top_builddir)/digikamimageplugins/common/widgets/libdigikamimagepluginswidget.la

digikamimageplugin_superimpose_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_superimpose.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_superimpose.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_superimpose_ui.rc

digikamimageplugin_superimposeicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_superimposeicon_ICON = AUTO
