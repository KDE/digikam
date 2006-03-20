METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/digikamimageplugins/common/include \
	       -I$(top_srcdir)/digikamimageplugins/common/widgets \
	       $(all_includes) $(LIBDIGIKAM_CFLAGS)

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_oilpaint.la 

digikamimageplugin_oilpaint_la_SOURCES = imageplugin_oilpaint.cpp \
	                                 imageeffect_oilpaint.cpp oilpaint.cpp 

digikamimageplugin_oilpaint_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS)\
	              $(top_builddir)/digikamimageplugins/common/widgets/libdigikamimagepluginswidget.la

digikamimageplugin_oilpaint_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_oilpaint.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_oilpaint.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_oilpaint_ui.rc        
       
digikamimageplugin_oilpainticondir = $(kde_datadir)/digikam/icons
digikamimageplugin_oilpainticon_ICON = AUTO
