METASOURCES = AUTO

noinst_LTLIBRARIES = libimagepluginswidgets.la

libimagepluginswidgets_la_SOURCES = imageregionwidget.cpp imagepaniconwidget.cpp imageselectionwidget.cpp \
	                            imageguidewidget.cpp imagepannelwidget.cpp imagewidget.cpp \
		                    listboxpreviewitem.cpp

libimagepluginswidgets_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
                 
INCLUDES = -I$(top_srcdir)/digikam/utilities/imageeditor/editor \
	   -I$(top_srcdir)/digikam/libs/dimg \
	   -I$(top_srcdir)/digikam/libs/dmetadata \
	   -I$(top_srcdir)/digikam/libs/widgets/common \
	   -I$(top_srcdir)/digikam/digikam \
	   $(all_includes)

digikaminclude_HEADERS = imageregionwidget.h imagepaniconwidget.h imagewidget.h \
	                 imageselectionwidget.h imagepannelwidget.h imageguidewidget.h \
	                 listboxpreviewitem.h

digikamincludedir = $(includedir)/digikam
