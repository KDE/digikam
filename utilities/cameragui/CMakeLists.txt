
include_directories( ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor ${CMAKE_SOURCE_DIR}/digikam/libs/jpegutils ${CMAKE_SOURCE_DIR}/digikam/libs/themeengine ${CMAKE_SOURCE_DIR}/digikam/libs/imageproperties ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/database ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############


########### install files ###############




#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libcameragui.la
#
## NOTE from Gilles (06-12-06): gpcamera.cpp must be placed on the top of source file list 
## to unbreak compilation with './configure -enable-final' option. I suspect a problem with 
## Gphoto2 C Ansi header.
#libcameragui_la_SOURCES = gpcamera.cpp cameraui.cpp cameraiconview.cpp  \
#	                  cameraiconitem.cpp cameracontroller.cpp \
#	                  camerafolderview.cpp camerafolderitem.cpp \
#	                  animwidget.cpp renamecustomizer.cpp  \
#	                  dkcamera.cpp umscamera.cpp gpiteminfo.cpp \
#	                  camerainfodialog.cpp albumselectdialog.cpp \
#	                  camerafolderdialog.cpp  
#
#libcameragui_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#libcameragui_la_LIBADD  = $(top_builddir)/digikam/libs/imageproperties/libimagepropertiescamgui.la \
#	                  $(LIB_GPHOTO) $(LIBJPEG)
#
#INCLUDES = -I$(top_srcdir)/digikam/digikam \
#	   -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
#	   -I$(top_srcdir)/digikam/libs/jpegutils \
#	   -I$(top_srcdir)/digikam/libs/themeengine \
#	   -I$(top_srcdir)/digikam/libs/imageproperties \
#	   $(LIBKEXIV2_CFLAGS) \
#	   -I$(top_srcdir)/digikam/libs/widgets/common \
#	   -I$(top_srcdir)/digikam/libs/dimg \
#	   -I$(top_srcdir)/digikam/libs/dmetadata \
#	   -I$(top_srcdir)/digikam/libs/database \
#	   $(GPHOTO_CFLAGS) $(LIBKDCRAW_CFLAGS) $(all_includes)
