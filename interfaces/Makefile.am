METASOURCES = AUTO

noinst_LTLIBRARIES = libdigikaminterfaces.la

libdigikaminterfaces_la_SOURCES = albummanager.cpp albuminfo.cpp \
		                  albumxmleditor.cpp albumxmlparser.cpp \
		                  digikamio.cpp \
		                  thumbnailjob.cpp albumitemhandler.cpp \
		                  kipiinterface.cpp

libdigikaminterfaces_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -lkipi

digikamincludedir = $(includedir)/digikam

INCLUDES= -I$(top_srcdir)/digikam/digikam \
	  $(all_includes) 

