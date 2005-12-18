METASOURCES = AUTO

noinst_LTLIBRARIES = libdialog.la

libdialog_la_SOURCES = ctrlpaneldlg.cpp imagedlgbase.cpp

libdialog_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/digikam \
	       -I$(top_srcdir)/digikam/libs/dimg/filters \
	       -I$(top_srcdir)/digikam/libs/widgets \
	       $(all_includes)

digikaminclude_HEADERS = ctrlpaneldlg.h imagedlgbase.h 
digikamincludedir = $(includedir)/digikam

