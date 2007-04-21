METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/digikam/digikam \
	   -I$(top_srcdir)/digikam/libs/dimg \
	   -I$(top_srcdir)/digikam/libs/thumbbar \
	   -I$(top_srcdir)/digikam/libs/dmetadata \
	   -I$(top_srcdir)/digikam/libs/themeengine \
	   -I$(top_srcdir)/digikam/libs/threadimageio \
	   $(all_includes)

noinst_LTLIBRARIES = liblighttable.la 

liblighttable_la_SOURCES = lighttablebar.cpp  

liblighttable_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)


