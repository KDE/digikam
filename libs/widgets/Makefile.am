METASOURCES = AUTO

noinst_LTLIBRARIES = libwidgets.la

libwidgets_la_SOURCES = imageregionwidget.cpp imagepreviewwidget.cpp imagepaniconwidget.cpp   

libwidgets_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = \
	-I$(top_srcdir)/digikam/utilities/imageeditor \
	$(all_includes) 

digikaminclude_HEADERS = imageregionwidget.h imagepreviewwidget.h imagepaniconwidget.h 

digikamincludedir = $(includedir)/digikam
