METASOURCES = AUTO

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      -I$(top_srcdir)/digikamimageplugins/common/cimgiface \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)   

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_restoration.la

digikamimageplugin_restoration_la_SOURCES = imageplugin_restoration.cpp \
	                                    imageeffect_restoration.cpp 

digikamimageplugin_restoration_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS) \
	                                   $(top_builddir)/digikamimageplugins/common/cimgiface/libcimgiface.la

digikamimageplugin_restoration_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) -no-undefined

kde_services_DATA = digikamimageplugin_restoration.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_restoration.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_restoration_ui.rc

digikamimageplugin_restorationicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_restorationicon_ICON = AUTO
