METASOURCES = AUTO

noinst_LTLIBRARIES = libimagepropertiesshowfoto.la libimagepropertiesdigikam.la \
	             libimagepropertiescamgui.la

# Image Properties SideBar for Camera GUI.

libimagepropertiescamgui_la_SOURCES = imagepropertiessidebarcamgui.cpp cameraitempropertiestab.cpp

libimagepropertiescamgui_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) 

# Image Properties SideBar for Showfoto (without digiKam database support).

libimagepropertiesshowfoto_la_SOURCES = imagepropertiessidebar.cpp navigatebarwidget.cpp \
	                                imagepropertiesmetadatatab.cpp imagepropertiescolorstab.cpp \
	                                imagepropertiestab.cpp navigatebartab.cpp

libimagepropertiesshowfoto_la_LIBADD = $(top_builddir)/digikam/libs/widgets/libwidgets.la \
	                               $(top_builddir)/digikam/libs/dmetadata/libdmetadata.la \
	                               $(top_builddir)/digikam/libs/dimg/libdimg.la \
	                               $(top_builddir)/digikam/libs/threadimageio/libthreadimageio.la \
	                               $(top_builddir)/digikam/libs/histogram/libhistogram.la 

libimagepropertiesshowfoto_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# Image Properties SideBar for digiKam Main interface and Image Editor (digiKam database support).

libimagepropertiesdigikam_la_SOURCES = imagedescedittab.cpp imagepropertiessidebar.cpp \
	                               imagepropertiessidebardb.cpp \
	                               talbumlistview.cpp imagepropertiesmetadatatab.cpp \
	                               imagepropertiescolorstab.cpp \
	                               navigatebarwidget.cpp imagepropertiestab.cpp navigatebartab.cpp

libimagepropertiesdigikam_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = -I$(top_srcdir)/digikam/libs/histogram \
	       -I$(top_srcdir)/digikam/libs/dcraw \
	       -I$(top_srcdir)/digikam/libs/themeengine \
	       -I$(top_srcdir)/digikam/libs/dmetadata \
	       -I$(top_srcdir)/digikam/libs/widgets/common \
	       -I$(top_srcdir)/digikam/libs/widgets/iccprofiles \
	       -I$(top_srcdir)/digikam/libs/widgets/metadata \
	       -I$(top_srcdir)/digikam/libs/dialogs \
	       -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/libs/threadimageio \
	       -I$(top_srcdir)/digikam/utilities/cameragui \
	       -I$(top_srcdir)/digikam/utilities/batch \
	       -I$(top_srcdir)/digikam/digikam \
	       $(LIBKEXIV2_CFLAGS) $(LIBKDCRAW_CFLAGS) \
	       $(all_includes)

