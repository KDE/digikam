METASOURCES = AUTO

noinst_LTLIBRARIES = libimagepropertiesshowfoto.la libimagepropertiesdigikam.la

# Image Properties SideBar for Showfoto (without digiKam database support).

libimagepropertiesshowfoto_la_SOURCES = imagepropertiessidebar.cpp navigatebarwidget.cpp \
	                                imagepropertiesexiftab.cpp imagepropertieshistogramtab.cpp 

libimagepropertiesshowfoto_la_LIBADD = $(top_builddir)/digikam/libs/widgets/libwidgets.la \
	                               $(top_builddir)/digikam/libs/histogram/libhistogram.la 

libimagepropertiesshowfoto_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# Image Properties SideBar for digiKam Main interface and Image Editor (digiKam database support).

libimagepropertiesdigikam_la_SOURCES = imagepropertiessidebardb.cpp \
	                               imagepropertiesexiftab.cpp imagepropertieshistogramtab.cpp \
	                               imagedescedittab.cpp navigatebarwidget.cpp 

libimagepropertiesdigikam_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = -I$(top_srcdir)/digikam/libs/histogram \
	   -I$(top_srcdir)/digikam/libs/widgets \
	   -I$(top_srcdir)/digikam/digikam \
	   $(LIBKEXIF_CFLAGS) $(all_includes)

digikamincludedir = $(includedir)/digikam
