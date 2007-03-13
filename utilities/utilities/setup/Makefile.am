METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/digikam/digikam \
	   -I$(top_srcdir)/digikam/utilities/batch \
	   -I$(top_srcdir)/digikam/libs/dialogs \
	   -I$(top_srcdir)/digikam/libs/dimg \
	   -I$(top_srcdir)/digikam/libs/dimg/loaders \
	   -I$(top_srcdir)/digikam/libs/dcraw \
	   -I$(top_srcdir)/digikam/libs/widgets/common \
	   $(GPHOTO_CFLAGS) $(LIBKIPI_CFLAGS) $(all_includes)

noinst_LTLIBRARIES = libsetup.la libshowfotosetup.la

# NOTE from Gilles (06-12-06): gpiface.cpp must be placed on the top of source file list 
# to unbreak compilation with './configure -enable-final' option. I suspect a problem with 
# Gphoto2 C Ansi header.
libsetup_la_SOURCES = gpiface.cpp cameraselection.cpp setupcamera.cpp \
	              setupmime.cpp setupplugins.cpp setupidentity.cpp \
	              setupgeneral.cpp setup.cpp setupimgplugins.cpp \
	              setupcollections.cpp setupmetadata.cpp \
	              setupeditor.cpp setupmisc.cpp setupicc.cpp \
	              setupiofiles.cpp setupdcraw.cpp setupslideshow.cpp \
	              setuptooltip.cpp  

libsetup_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_GPHOTO)

libshowfotosetup_la_SOURCES = setupimgplugins.cpp setupiofiles.cpp setupdcraw.cpp \
	                      setupicc.cpp setupslideshow.cpp 

libshowfotosetup_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) 

