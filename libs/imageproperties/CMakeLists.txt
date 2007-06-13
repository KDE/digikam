INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/histogram ${CMAKE_SOURCE_DIR}/digikam/libs/dcraw ${CMAKE_SOURCE_DIR}/digikam/libs/themeengine ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/metadata ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/threadimageio ${CMAKE_SOURCE_DIR}/digikam/libs/database ${CMAKE_SOURCE_DIR}/digikam/utilities/cameragui ${CMAKE_SOURCE_DIR}/digikam/utilities/batch ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(imagepropertiesshowfoto_STAT_SRCS
   imagepropertiessidebar.cpp
   navigatebarwidget.cpp
   imagepropertiesmetadatatab.cpp
   imagepropertiescolorstab.cpp
   imagepropertiestab.cpp
   navigatebartab.cpp
   )

KDE3_AUTOMOC(${imagepropertiesshowfoto_STAT_SRCS})

ADD_LIBRARY(imagepropertiesshowfoto STATIC ${imagepropertiesshowfoto_STAT_SRCS})


########### next target ###############

SET(imagepropertiesdigikam_STAT_SRCS
   imagedescedittab.cpp
   imagepropertiessidebar.cpp
   imagepropertiessidebardb.cpp
   talbumlistview.cpp
   imagepropertiesmetadatatab.cpp
   imagepropertiescolorstab.cpp
   navigatebarwidget.cpp
   imagepropertiestab.cpp
   navigatebartab.cpp
   )

KDE3_AUTOMOC(${imagepropertiesdigikam_STAT_SRCS})

ADD_LIBRARY(imagepropertiesdigikam STATIC ${imagepropertiesdigikam_STAT_SRCS})


########### next target ###############

SET(imagepropertiescamgui_STAT_SRCS
   imagepropertiessidebarcamgui.cpp
   cameraitempropertiestab.cpp
   )

KDE3_AUTOMOC(${imagepropertiescamgui_STAT_SRCS})

ADD_LIBRARY(imagepropertiescamgui STATIC ${imagepropertiescamgui_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libimagepropertiesshowfoto.la libimagepropertiesdigikam.la \
#	             libimagepropertiescamgui.la
#
## Image Properties SideBar for Camera GUI.
#
#libimagepropertiescamgui_la_SOURCES = imagepropertiessidebarcamgui.cpp cameraitempropertiestab.cpp
#
#libimagepropertiescamgui_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
#
## Image Properties SideBar for Showfoto (without digiKam database support).
#
#libimagepropertiesshowfoto_la_SOURCES = imagepropertiessidebar.cpp navigatebarwidget.cpp \
#	                                imagepropertiesmetadatatab.cpp imagepropertiescolorstab.cpp \
#	                                imagepropertiestab.cpp navigatebartab.cpp
#
#libimagepropertiesshowfoto_la_LIBADD = $(top_builddir)/digikam/libs/widgets/libwidgets.la \
#	                               $(top_builddir)/digikam/libs/dmetadata/libdmetadata.la \
#	                               $(top_builddir)/digikam/libs/dimg/libdimg.la \
#	                               $(top_builddir)/digikam/libs/threadimageio/libthreadimageio.la \
#	                               $(top_builddir)/digikam/libs/histogram/libhistogram.la 
#
#libimagepropertiesshowfoto_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
## Image Properties SideBar for digiKam Main interface and Image Editor (digiKam database support).
#
#libimagepropertiesdigikam_la_SOURCES = imagedescedittab.cpp imagepropertiessidebar.cpp \
#	                               imagepropertiessidebardb.cpp \
#	                               talbumlistview.cpp imagepropertiesmetadatatab.cpp \
#	                               imagepropertiescolorstab.cpp \
#	                               navigatebarwidget.cpp imagepropertiestab.cpp navigatebartab.cpp
#
#libimagepropertiesdigikam_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#INCLUDES = -I$(top_srcdir)/digikam/libs/histogram \
#	       -I$(top_srcdir)/digikam/libs/dcraw \
#	       -I$(top_srcdir)/digikam/libs/themeengine \
#	       -I$(top_srcdir)/digikam/libs/dmetadata \
#	       -I$(top_srcdir)/digikam/libs/widgets/common \
#	       -I$(top_srcdir)/digikam/libs/widgets/iccprofiles \
#	       -I$(top_srcdir)/digikam/libs/widgets/metadata \
#	       -I$(top_srcdir)/digikam/libs/dialogs \
#	       -I$(top_srcdir)/digikam/libs/dimg \
#	       -I$(top_srcdir)/digikam/libs/threadimageio \
#	       -I$(top_srcdir)/digikam/libs/database \
#	       -I$(top_srcdir)/digikam/utilities/cameragui \
#	       -I$(top_srcdir)/digikam/utilities/batch \
#	       -I$(top_srcdir)/digikam/digikam \
#	       $(LIBKEXIV2_CFLAGS) $(LIBKDCRAW_CFLAGS) \
#	       $(all_includes)
#
