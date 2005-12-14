METASOURCES = AUTO

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      -I$(top_srcdir)/digikamimageplugins/common/cimgiface \
	      -I$(top_srcdir)/digikamimageplugins/common/widgets \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)   

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_inpainting.la

digikamimageplugin_inpainting_la_SOURCES = imageplugin_inpainting.cpp \
	                                    imageeffect_inpainting.cpp 

digikamimageplugin_inpainting_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS) \
	              $(top_builddir)/digikamimageplugins/common/cimgiface/libcimgiface.la \
	              $(top_builddir)/digikamimageplugins/common/widgets/libdigikamimagepluginswidget.la 

digikamimageplugin_inpainting_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) -no-undefined

kde_services_DATA = digikamimageplugin_inpainting.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_inpainting.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_inpainting_ui.rc        

digikamimageplugin_inpaintingicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_inpaintingicon_ICON = AUTO
