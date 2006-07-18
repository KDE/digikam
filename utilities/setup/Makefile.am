METASOURCES = AUTO

INCLUDES= -I$(top_srcdir)/digikam/digikam \
	      -I$(top_srcdir)/digikam/libs/dialogs \
	      -I$(top_srcdir)/digikam/libs/dimg \
          -I$(top_srcdir)/digikam/libs/widgets/common \
	      $(GPHOTO_CFLAGS) $(LIBKIPI_CFLAGS) \
	      $(all_includes)

noinst_LTLIBRARIES = libsetup.la libshowfotosetup.la

libsetup_la_SOURCES = setup.cpp setupcamera.cpp setupimgplugins.cpp \
	                  cameraselection.cpp setupgeneral.cpp \
	                  gpiface.cpp setupmime.cpp setupplugins.cpp \
	                  setupcollections.cpp setupmetadata.cpp \
	                  setupeditor.cpp setupmisc.cpp setupicc.cpp \
	                  setupiofiles.cpp setupslideshow.cpp setupidentity.cpp 

libsetup_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_GPHOTO)

libshowfotosetup_la_SOURCES = setupimgplugins.cpp setupiofiles.cpp \
	                          setupicc.cpp setupslideshow.cpp

libshowfotosetup_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) 

KDE_OPTIONS=nofinal
