METASOURCES = AUTO

noinst_LTLIBRARIES = libthemeengine.la

libthemeengine_la_SOURCES = theme.cpp themeengine.cpp texture.cpp

libthemeengine_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
 	       -I$(top_srcdir)/digikam/digikam \
	       -I$(top_srcdir)/digikam/ \
	       $(all_includes)