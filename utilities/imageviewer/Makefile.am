METASOURCES = AUTO

noinst_LTLIBRARIES = libimageviewer.la

libimageviewer_la_SOURCES = imageview.cpp canvas.cpp \
		            imlibinterface.cpp exifrestorer.cpp \
		            imeventfilter.cpp imagebcgedit.cpp

libimageviewer_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_IMLIB)

INCLUDES= -I$(top_srcdir)/digikam/digikam \
	  -I$(top_srcdir)/digikam/interfaces \
	  -I$(top_srcdir)/digikam/utilities/kexif \
	  $(IMLIB_CFLAGS) $(all_includes) 

digikamimageviewericondir = $(kde_datadir)/digikam/icons
digikamimageviewericon_ICON = AUTO