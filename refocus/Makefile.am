SUBDIRS = clapack 
COMPILE_FIRST = clapack 

METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikamimageplugins/common/include \
	      -I$(top_srcdir)/digikamimageplugins/common/widgets \
	      -I$(top_srcdir)/digikamimageplugins/refocus/clapack \
	      -I$(top_srcdir)/digikam/digikam \
	      $(all_includes) $(LIBDIGIKAM_CFLAGS)   
              
# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_refocus.la

digikamimageplugin_refocus_la_SOURCES = imageplugin_refocus.cpp imageeffect_refocus.cpp \
	                                matrix.cpp refocus.cpp 

digikamimageplugin_refocus_la_LIBADD = $(LIB_KPARTS) $(LIBDIGIKAM_LIBS) \
	              $(top_builddir)/digikamimageplugins/refocus/clapack/liblapack.la \
	              $(top_builddir)/digikamimageplugins/common/widgets/libdigikamimagepluginswidget.la

digikamimageplugin_refocus_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_refocus.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_refocus.pot

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_refocus_ui.rc        

digikamimageplugin_refocusicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_refocusicon_ICON = AUTO
