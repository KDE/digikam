METASOURCES = AUTO

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      -I$(top_srcdir)/digikamimageplugins/common/cimgiface \
	      -I$(top_srcdir)/digikamimageplugins/common/widgets \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)   

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_blowup.la

digikamimageplugin_blowup_la_SOURCES = imageplugin_blowup.cpp \
	                                    imageeffect_blowup.cpp 

digikamimageplugin_blowup_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS) \
	              $(top_builddir)/digikamimageplugins/common/cimgiface/libcimgiface.la \
	              $(top_builddir)/digikamimageplugins/common/widgets/libdigikamimagepluginswidget.la 

digikamimageplugin_blowup_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_blowup.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_blowup.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_blowup_ui.rc        

digikamimageplugin_blowupicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_blowupicon_ICON = AUTO
