METASOURCES = AUTO

noinst_LTLIBRARIES = libdatabase.la

libdatabase_la_SOURCES = albumdb.cpp \
	                 collectionscanner.cpp \
	                 databaseaccess.cpp \
	                 databaseattributeswatch.cpp \
	                 databaseparameters.cpp \
	                 databasetransaction.cpp\
	                 databaseurl.cpp \
	                 imageinfo.cpp \
	                 imageinfolist.cpp \
	                 imagelister.cpp \
	                 imagelisterreceiver.cpp \
	                 imagequerybuilder.cpp

libdatabase_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

#libdmetadata_la_LIBADD  = $(LIBKEXIV2_LIBS) $(LIBKDCRAW_LIBS) $(LIB_KIO) \
#	                  $(LIB_SQLITE3)


INCLUDES = $(LIBKEXIV2_CFLAGS) $(LIBKDCRAW_CFLAGS) $(all_includes) \
	       -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/libs/dmetadata \
	       -I$(top_srcdir)/digikam/libs/debug


digikaminclude_HEADERS = albumdb.h
digikamincludedir = $(includedir)/digikam
