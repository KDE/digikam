METASOURCES = AUTO

INCLUDES    = -I$(top_srcdir)/digikam/utilities/imageeditor \
	-I$(top_srcdir)/digikam/libs/guibuilder \
	-I$(top_srcdir)/digikam/libs/widgets \
	$(all_includes)   
        
# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = digikamimageplugin_oilpaint.la

digikamimageplugin_oilpaint_la_SOURCES = \
	imageplugin_oilpaint.cpp \
	imageeffect_oilpaint.cpp

digikamimageplugin_oilpaint_la_LIBADD = $(LIB_KPARTS) \
	$(top_builddir)/digikam/digikam/libdigikam.la

digikamimageplugin_oilpaint_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

kde_services_DATA = digikamimageplugin_oilpaint.desktop

# i18n translation messages
messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/digikamimageplugin_oilpaint.pot

