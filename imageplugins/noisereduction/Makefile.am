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

digikamimageplugin_despeckle_la_SOURCES = imageplugin_noisereduction.cpp \
	                                      imageeffect_noisereduction.cpp noisereduction.cpp

digikamimageplugin_despeckle_la_LIBADD = $(LIB_KPARTS) \
	                                     $(top_builddir)/digikam/digikam/libdigikam.la 

digikamimageplugin_despeckle_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries) 

kde_services_DATA = digikamimageplugin_despeckle.desktop

kde_module_LTLIBRARIES = digikamimageplugin_despeckle.la

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_despeckle_ui.rc
       
digikamimageplugin_despeckleicondir = $(kde_datadir)/digikam/icons
digikamimageplugin_despeckleicon_ICON = AUTO
