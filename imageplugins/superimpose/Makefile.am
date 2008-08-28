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
	       -I$(top_srcdir)/digikam/libs/thumbbar \
	       -I$(top_srcdir)/digikam/digikam \
	       $(LIBKDCRAW_CFLAGS) \
	       $(all_includes) 

digikamimageplugin_superimpose_la_SOURCES = superimposewidget.cpp superimpose.cpp dirselectwidget.cpp \
	                                        imageplugin_superimpose.cpp superimposetool.cpp 
	                                        

digikamimageplugin_superimpose_la_LIBADD = $(LIB_KPARTS) \
	                                       $(top_builddir)/digikam/digikam/libdigikam.la 

digikamimageplugin_superimpose_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_superimpose.desktop

kde_module_LTLIBRARIES = digikamimageplugin_superimpose.la

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_superimpose_ui.rc
