METASOURCES = AUTO

noinst_LTLIBRARIES = libdialog.la libdialogshowfoto.la

# Dialogs collection used by Showfoto.

libdialogshowfoto_la_SOURCES = iccprofileinfodlg.cpp imagedialog.cpp rawcameradlg.cpp

libdialogshowfoto_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# Dialogs collection used by digiKam.

libdialog_la_SOURCES = deletedialogbase.ui imagedialog.cpp rawcameradlg.cpp \
	                   iccprofileinfodlg.cpp deletedialog.cpp dprogressdlg.cpp 

libdialog_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = -I$(top_srcdir)/digikam/digikam \
	       -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/libs/dmetadata \
	       -I$(top_srcdir)/digikam/libs/thumbbar \
	       -I$(top_srcdir)/digikam/libs/dimg/filters \
	       -I$(top_srcdir)/digikam/libs/widgets/common \
	       -I$(top_srcdir)/digikam/libs/widgets/metadata \
	       -I$(top_srcdir)/digikam/libs/widgets/iccprofiles \
	       -I$(top_srcdir)/digikam/libs/widgets/imageplugins \
	       -I$(top_srcdir)/digikam/utilities/imageeditor/canvas \
	       $(LIBKDCRAW_CFLAGS) \
	       $(LIBKEXIV2_CFLAGS) \
	       $(all_includes)

digikaminclude_HEADERS = iccprofileinfodlg.h dprogressdlg.h imagedialog.h rawcameradlg.h 
digikamincludedir = $(includedir)/digikam
