SUBDIRS = sharpnesseditor hsl ratiocrop 
COMPILE_FIRST = sharpnesseditor hsl ratiocrop 
METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
	       -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
	       -I$(top_srcdir)/digikam/libs/histogram \
	       -I$(top_srcdir)/digikam/libs/levels \
	       -I$(top_srcdir)/digikam/libs/curves \
	       -I$(top_srcdir)/digikam/libs/whitebalance \
	       -I$(top_srcdir)/digikam/libs/widgets/common \
	       -I$(top_srcdir)/digikam/libs/widgets/iccprofiles \
	       -I$(top_srcdir)/digikam/libs/widgets/imageplugins \
	       -I$(top_srcdir)/digikam/libs/dialogs \
	       -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/libs/dmetadata \
	       -I$(top_srcdir)/digikam/libs/dimg/filters \
	       -I$(top_srcdir)/digikam/digikam \
	       -I$(top_srcdir)/digikam/imageplugins/coreplugin/sharpnesseditor \
	       -I$(top_srcdir)/digikam/imageplugins/coreplugin/hsl \
	       -I$(top_srcdir)/digikam/imageplugins/coreplugin/ratiocrop \
	       $(LIBKDCRAW_CFLAGS) \
	       $(all_includes) 

digikamimageplugin_core_la_SOURCES = imageplugin_core.cpp bwsepiatool.cpp \
	                                 autocorrectiontool.cpp \
	                                 rgbtool.cpp \
	                                 redeyetool.cpp blurtool.cpp \
	                                 iccprooftool.cpp bcgtool.cpp 

noinst_HEADERS = autocorrectiontool.h blurtool.h \
                 rgbtool.h bcgtool.h \
                 bwsepiatool.h iccprooftool.h redeyetool.h 

digikamimageplugin_core_la_LIBADD = $(LIB_KPARTS) \
                                    $(top_builddir)/digikam/imageplugins/coreplugin/sharpnesseditor/libsharpnesseditor.la \
	                            $(top_builddir)/digikam/imageplugins/coreplugin/hsl/libhsl.la \
	                            $(top_builddir)/digikam/imageplugins/coreplugin/ratiocrop/libratiocrop.la \
	                            $(top_builddir)/digikam/digikam/libdigikam.la

digikamimageplugin_core_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

kde_services_DATA = digikamimageplugin_core.desktop

kde_module_LTLIBRARIES = digikamimageplugin_core.la

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_core_ui.rc
