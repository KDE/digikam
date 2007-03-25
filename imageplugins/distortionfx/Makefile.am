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
	       $(all_includes) 

digikamimageplugin_distortionfx_la_SOURCES = imageplugin_distortionfx.cpp \
	                                         imageeffect_distortionfx.cpp distortionfx.cpp

digikamimageplugin_distortionfx_la_LIBADD = $(LIB_KPARTS) \
	                                        $(top_builddir)/digikam/digikam/libdigikam.la

digikamimageplugin_distortionfx_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_distortionfx.desktop

kde_module_LTLIBRARIES = digikamimageplugin_distortionfx.la 

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_distortionfx_ui.rc     
       
