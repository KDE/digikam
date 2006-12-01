METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/digikam/utilities/setup \
	       $(all_includes)

noinst_LTLIBRARIES = libsetup.la

libsetup_la_SOURCES = setup.cpp setupeditor.cpp

libsetup_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) 

libsetup_la_LIBADD = $(top_builddir)/digikam/utilities/setup/libshowfotosetup.la \
	                 $(all_libraries) $(KDE_RPATH)

