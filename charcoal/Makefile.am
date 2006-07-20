METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
		  -I$(top_srcdir)/digikam/digikam \
	      -I$(top_srcdir)/digikamimageplugins/common/dialogs \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_charcoal.la

digikamimageplugin_charcoal_la_SOURCES = imageplugin_charcoal.cpp \
	                                 imageeffect_charcoal.cpp charcoal.cpp 

digikamimageplugin_charcoal_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS) \
	              $(top_builddir)/digikamimageplugins/common/dialogs/libdigikamimagepluginsdialog.la 

digikamimageplugin_charcoal_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_charcoal.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_charcoal.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_charcoal_ui.rc
       
digikamimageplugin_charcoalicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_charcoalicon_ICON = AUTO
