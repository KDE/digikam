METASOURCES = AUTO

noinst_LTLIBRARIES = libimagepluginswidgets.la

libimagepluginswidgets_la_SOURCES = imageregionwidget.cpp imagepaniconwidget.cpp imageguidewidget.cpp \
	                                imagewidget.cpp listboxpreviewitem.cpp imagepanelwidget.cpp

libimagepluginswidgets_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
                 
INCLUDES = -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
	       -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/libs/dmetadata \
	       -I$(top_srcdir)/digikam/libs/widgets/common \
	       -I$(top_srcdir)/digikam/digikam \
		$(LIBKDCRAW_CFLAGS) \
		$(all_includes)

digikaminclude_HEADERS = imageregionwidget.h imagepaniconwidget.h \
	                     imagepanelwidget.h imageguidewidget.h \
	                     listboxpreviewitem.h imagewidget.h 

digikamincludedir = $(includedir)/digikam
