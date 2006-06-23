METASOURCES = AUTO

noinst_LTLIBRARIES = libiccprofileswidgets.la

libiccprofileswidgets_la_SOURCES = iccprofilewidget.cpp cietonguewidget.cpp iccpreviewwidget.cpp 

libiccprofileswidgets_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

libiccprofileswidgets_la_LIBADD = $(top_builddir)/digikam/libs/lprof/liblprof.la 
                 
INCLUDES = -I$(top_srcdir)/digikam/libs/widgets/metadata \
	   -I$(top_srcdir)/digikam/libs/widgets/common \
	   -I$(top_srcdir)/digikam/libs/lprof \
	   -I$(top_srcdir)/digikam/digikam \
	   $(all_includes)
           
digikaminclude_HEADERS = cietonguewidget.h iccprofilewidget.h iccpreviewwidget.h 

digikamincludedir = $(includedir)/digikam
