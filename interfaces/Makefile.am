METASOURCES = AUTO

noinst_LTLIBRARIES = libdigikaminterfaces.la

libdigikaminterfaces_la_SOURCES = plugin.cpp \
		albummanager.cpp albuminfo.cpp \
		albumxmleditor.cpp albumxmlparser.cpp \
		digikamio.cpp \
		thumbnailjob.cpp albumitemhandler.cpp

libdigikaminterfaces_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

digikaminclude_HEADERS = plugin.h albummanager.h albuminfo.h \
			digikamio.h thumbnailjob.h

digikamincludedir = $(includedir)/digikam

INCLUDES= -I$(top_srcdir)/digikam \
	$(all_includes) 

kde_servicetypes_DATA = digikamplugin.desktop
