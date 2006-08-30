METASOURCES = AUTO

noinst_LTLIBRARIES = libdialog.la

libdialog_la_SOURCES = ctrlpaneldlg.cpp imagedlgbase.cpp imageguidedlg.cpp \
	                   iccprofileinfodlg.cpp deletedialogbase.ui deletedialog.cpp \
			   dprogressdlg.cpp

libdialog_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/digikam \
	       -I$(top_srcdir)/digikam/libs/dimg/filters \
	       -I$(top_srcdir)/digikam/libs/widgets/common \
	       -I$(top_srcdir)/digikam/libs/widgets/metadata \
	       -I$(top_srcdir)/digikam/libs/widgets/iccprofiles \
	       -I$(top_srcdir)/digikam/libs/widgets/imageplugins \
	       -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
	       $(all_includes)

digikaminclude_HEADERS = ctrlpaneldlg.h imagedlgbase.h imageguidedlg.h iccprofileinfodlg.h dprogressdlg.h 
digikamincludedir = $(includedir)/digikam

