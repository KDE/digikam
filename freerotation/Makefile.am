METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/digikamimageplugins/common/include \
	       -I$(top_srcdir)/digikamimageplugins/common/widgets \
	       $(all_includes) $(LIBDIGIKAM_CFLAGS)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_freerotation.la

digikamimageplugin_freerotation_la_SOURCES = imageplugin_freerotation.cpp \
	                                         imageeffect_freerotation.cpp freerotation.cpp 

digikamimageplugin_freerotation_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS) \
	              $(top_builddir)/digikamimageplugins/common/widgets/libdigikamimagepluginswidget.la

digikamimageplugin_freerotation_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

kde_services_DATA = digikamimageplugin_freerotation.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_freerotation.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_freerotation_ui.rc

digikamimageplugin_freerotationicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_freerotationicon_ICON = AUTO
