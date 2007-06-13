INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/utilities/batch ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/loaders ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(setup_STAT_SRCS
   gpiface.cpp
   cameraselection.cpp
   setupcamera.cpp
   setupmime.cpp
   setupplugins.cpp
   setupidentity.cpp
   setupgeneral.cpp
   setup.cpp
   setupcollections.cpp
   setupmetadata.cpp
   setupeditor.cpp
   setupmisc.cpp
   setupicc.cpp
   setupiofiles.cpp
   setupdcraw.cpp
   setupslideshow.cpp
   setuptooltip.cpp
   setuplighttable.cpp
   )

KDE3_AUTOMOC(${setup_STAT_SRCS})

ADD_LIBRARY(setup STATIC ${setup_STAT_SRCS})


########### next target ###############

SET(showfotosetup_STAT_SRCS
   setupiofiles.cpp
   setupdcraw.cpp
   setupicc.cpp
   setupslideshow.cpp
   )

KDE3_AUTOMOC(${showfotosetup_STAT_SRCS})

ADD_LIBRARY(showfotosetup STATIC ${showfotosetup_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#INCLUDES = -I$(top_srcdir)/digikam/digikam \
#	   -I$(top_srcdir)/digikam/utilities/batch \
#	   -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
#	   -I$(top_srcdir)/digikam/libs/dialogs \
#	   -I$(top_srcdir)/digikam/libs/dimg \
#	   -I$(top_srcdir)/digikam/libs/dimg/loaders \
#	   -I$(top_srcdir)/digikam/libs/widgets/common \
#	   $(GPHOTO_CFLAGS) $(LIBKIPI_CFLAGS) $(LIBKDCRAW_CFLAGS) $(all_includes)
#
#noinst_LTLIBRARIES = libsetup.la libshowfotosetup.la
#
## NOTE from Gilles (06-12-06): gpiface.cpp must be placed on the top of source file list 
## to unbreak compilation with './configure -enable-final' option. I suspect a problem with 
## Gphoto2 C Ansi header.
#libsetup_la_SOURCES = gpiface.cpp cameraselection.cpp setupcamera.cpp \
#	              setupmime.cpp setupplugins.cpp setupidentity.cpp \
#	              setupgeneral.cpp setup.cpp \
#	              setupcollections.cpp setupmetadata.cpp \
#	              setupeditor.cpp setupmisc.cpp setupicc.cpp \
#	              setupiofiles.cpp setupdcraw.cpp setupslideshow.cpp \
#	              setuptooltip.cpp setuplighttable.cpp 
#
#libsetup_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_GPHOTO)
#
#libshowfotosetup_la_SOURCES = setupiofiles.cpp setupdcraw.cpp \
#	                      setupicc.cpp setupslideshow.cpp 
#
#libshowfotosetup_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
#
