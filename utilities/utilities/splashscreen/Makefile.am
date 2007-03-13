METASOURCES = AUTO

noinst_LTLIBRARIES = libsplashscreen.la

libsplashscreen_la_SOURCES = splashscreen.cpp

libsplashscreen_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

digikamsplashpicdir = $(kde_datadir)/digikam
digikamsplashpic_DATA = digikam-splash.png

INCLUDES= -I$(top_srcdir)/digikam/digikam \
	  $(all_includes)
