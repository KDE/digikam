METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
	       -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
	       -I$(top_srcdir)/digikam/libs/histogram \
	       -I$(top_srcdir)/digikam/libs/levels \
	       -I$(top_srcdir)/digikam/libs/curves \
	       -I$(top_srcdir)/digikam/libs/widgets/common \
	       -I$(top_srcdir)/digikam/libs/widgets/iccprofiles \
	       -I$(top_srcdir)/digikam/libs/widgets/imageplugins \
	       -I$(top_srcdir)/digikam/libs/dialogs \
	       -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/libs/dmetadata \
	       -I$(top_srcdir)/digikam/libs/dimg/filters \
	       -I$(top_srcdir)/digikam/digikam \
	       $(all_includes) 

digikamimageplugin_core_la_SOURCES = imageplugin_core.cpp  imageeffect_sharpen.cpp \
	                                 imageeffect_autocorrection.cpp imageeffect_bwsepia.cpp \
	                                 imageeffect_ratiocrop.cpp imageeffect_rgb.cpp \
	                                 imageeffect_redeye.cpp imageeffect_blur.cpp \
	                                 imageeffect_hsl.cpp imageeffect_iccproof.cpp imageeffect_bcg.cpp 

noinst_HEADERS = imageeffect_autocorrection.h imageeffect_blur.h imageeffect_hsl.h \
                 imageeffect_ratiocrop.h imageeffect_rgb.h imageplugin_core.h imageeffect_bcg.h \
                 imageeffect_bwsepia.h imageeffect_iccproof.h imageeffect_redeye.h imageeffect_sharpen.h 

digikamimageplugin_core_la_LIBADD = $(LIB_KPARTS) \
	                                $(top_builddir)/digikam/digikam/libdigikam.la

digikamimageplugin_core_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)

kde_services_DATA = digikamimageplugin_core.desktop

kde_module_LTLIBRARIES = digikamimageplugin_core.la

rcdir = $(kde_datadir)/digikam
rc_DATA = digikamimageplugin_core_ui.rc

