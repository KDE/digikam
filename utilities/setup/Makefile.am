METASOURCES = AUTO

INCLUDES= -I$(top_srcdir)/digikam/digikam \
	    $(all_includes) \
	    $(GPHOTO_CFLAGS)

noinst_LTLIBRARIES = libsetup.la

libsetup_la_SOURCES = setup.cpp setupcamera.cpp \
	              cameraselection.cpp setupgeneral.cpp \
	              gpiface.cpp setupmime.cpp setupplugins.cpp \
		      setupcollections.cpp setupexif.cpp \
		      setupeditor.cpp setupmisc.cpp

libsetup_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_GPHOTO)

