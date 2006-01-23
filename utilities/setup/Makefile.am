METASOURCES = AUTO

INCLUDES= -I$(top_srcdir)/digikam/digikam \
	    $(all_includes) \
	    $(GPHOTO_CFLAGS) $(LIBKIPI_CFLAGS)

noinst_LTLIBRARIES = libsetup.la libshowfotosetup.la

libsetup_la_SOURCES = setup.cpp setupcamera.cpp setupimgplugins.cpp \
	              cameraselection.cpp setupgeneral.cpp \
	              gpiface.cpp setupmime.cpp setupplugins.cpp \
		      setupcollections.cpp setupexif.cpp \
		      setupeditor.cpp setupmisc.cpp setupicc.cpp \
		      setupiofiles.cpp 

libsetup_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_GPHOTO)

libshowfotosetup_la_SOURCES = setupimgplugins.cpp setupiofiles.cpp 

libshowfotosetup_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) 

KDE_OPTIONS=nofinal
