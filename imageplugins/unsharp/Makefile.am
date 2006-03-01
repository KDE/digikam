METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      -I$(top_srcdir)/digikamimageplugins/common/widgets \
	      -I$(top_srcdir)/digikam/digikam \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)   

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_unsharp.la

digikamimageplugin_unsharp_la_SOURCES = imageplugin_unsharp.cpp \
	                                imageeffect_unsharp.cpp unsharp.cpp 

digikamimageplugin_unsharp_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS)\
	              $(top_builddir)/digikamimageplugins/common/widgets/libdigikamimagepluginswidget.la

digikamimageplugin_unsharp_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_unsharp.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_unsharp.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_unsharp_ui.rc
       
digikamimageplugin_unsharpicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_unsharpicon_ICON = AUTO
