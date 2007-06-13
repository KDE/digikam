ADD_SUBDIRECTORY( pics ) 
ADD_SUBDIRECTORY( icons ) 
ADD_SUBDIRECTORY( setup ) 

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/showfoto/setup ${CMAKE_SOURCE_DIR}/digikam/utilities/setup ${CMAKE_SOURCE_DIR}/digikam/libs/thumbbar ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/imageproperties ${CMAKE_SOURCE_DIR}/digikam/libs/histogram ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/threadimageio ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/utilities/slideshow ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/tools ${CMAKE_SOURCE_DIR}/digikam/utilities/splashscreen ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(showfoto_SRCS
   main.cpp
   showfoto.cpp
   )

KDE3_AUTOMOC(${showfoto_SRCS})

KDE3_ADD_EXECUTABLE(showfoto ${showfoto_SRCS})

TARGET_LINK_LIBRARIES(showfoto  ${QT_AND_KDECORE_LIBS} setup dialogshowfoto imagepropertiesshowfoto thumbbar showfoto splashscreen slideshow threadimageio jpegutils kutils kparts )

INSTALL( TARGETS showfoto DESTINATION bin)


########### install files ###############

INSTALL_FILES( /share/applications/kde FILES  showfoto.desktop )
INSTALL_FILES( /share/apps/showfoto FILES  showfotoui.rc )



#original Makefile.am contents follow:

#SUBDIRS = pics icons setup 
#
#METASOURCES = AUTO  
#
#bin_PROGRAMS = showfoto
#
#INCLUDES = -I$(top_srcdir)/digikam/showfoto/setup \
#	   -I$(top_srcdir)/digikam/utilities/setup \
#	   -I$(top_srcdir)/digikam/libs/thumbbar \
#	   -I$(top_srcdir)/digikam/libs/widgets/common \
#	   -I$(top_srcdir)/digikam/libs/imageproperties \
#	   -I$(top_srcdir)/digikam/libs/histogram \
#	   -I$(top_srcdir)/digikam/libs/dimg \
#	   -I$(top_srcdir)/digikam/libs/dmetadata \
#	   -I$(top_srcdir)/digikam/libs/threadimageio \
#	   -I$(top_srcdir)/digikam/digikam \
#	   -I$(top_srcdir)/digikam/utilities/slideshow \
#	   -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
#	   -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
#	   -I$(top_srcdir)/digikam/utilities/imageeditor/tools \
#	   -I$(top_srcdir)/digikam/utilities/splashscreen \
#	   $(LIBKEXIV2_CFLAGS) $(LIBKDCRAW_CFLAGS) $(all_includes)
#
#showfoto_SOURCES = main.cpp showfoto.cpp 
#
#showfoto_LDADD   = $(top_builddir)/digikam/showfoto/setup/libsetup.la \
#	           $(top_builddir)/digikam/libs/dialogs/libdialogshowfoto.la \
#	           $(top_builddir)/digikam/libs/imageproperties/libimagepropertiesshowfoto.la \
#	           $(top_builddir)/digikam/libs/thumbbar/libthumbbar.la \
#	           $(top_builddir)/digikam/utilities/imageeditor/editor/libshowfoto.la \
#	           $(top_builddir)/digikam/utilities/splashscreen/libsplashscreen.la \
#	           $(top_builddir)/digikam/utilities/slideshow/libslideshow.la \
#	           $(top_builddir)/digikam/libs/threadimageio/libthreadimageio.la \
#	           $(top_builddir)/digikam/libs/jpegutils/libjpegutils.la \
#	           $(LIB_KUTILS) $(LIB_KPARTS) $(LIBJPEG)  
#
#showfoto_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#
#rcdir = $(kde_datadir)/showfoto
#rc_DATA = showfotoui.rc
#
#xdg_apps_DATA = showfoto.desktop
