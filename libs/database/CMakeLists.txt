INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/debug ${CMAKE_SOURCE_DIR}/digikam/sqlite ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(database_STAT_SRCS
   albumdb.cpp
   albumdb_sqlite2.cpp
   collectionscanner.cpp
   collectionmanager.cpp
   collectionlocation.cpp
   databaseaccess.cpp
   databaseattributeswatch.cpp
   databasebackend.cpp
   databaseparameters.cpp
   databasetransaction.cpp
   databaseurl.cpp
   imageinfo.cpp
   imageinfolist.cpp
   imageinfocache.cpp
   imagelister.cpp
   imagelisterreceiver.cpp
   imagequerybuilder.cpp
   namefilter.cpp
   schemaupdater.cpp
   backendsqlite3.cpp
   upgradedb_sqlite2tosqlite3.cpp
   )

KDE3_AUTOMOC(${database_STAT_SRCS})

ADD_LIBRARY(database STATIC ${database_STAT_SRCS})


########### install files ###############

INSTALL_FILES( /include/digikam FILES  albumdb.h )



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libdatabase.la
#
#libdatabase_la_SOURCES = albumdb.cpp \
#	                 albumdb_sqlite2.cpp \
#	                 collectionscanner.cpp \
#	                 collectionmanager.cpp \
#	                 collectionlocation.cpp \
#	                 databaseaccess.cpp \
#	                 databaseattributeswatch.cpp \
#	                 databasebackend.cpp \
#	                 databaseparameters.cpp \
#	                 databasetransaction.cpp\
#	                 databaseurl.cpp \
#	                 imageinfo.cpp \
#	                 imageinfolist.cpp \
#	                 imageinfocache.cpp \
#	                 imagelister.cpp \
#	                 imagelisterreceiver.cpp \
#	                 imagequerybuilder.cpp \
#	                 namefilter.cpp \
#	                 schemaupdater.cpp \
#	                 backendsqlite3.cpp \
#	                 upgradedb_sqlite2tosqlite3.cpp 
#
#libdatabase_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
##libdmetadata_la_LIBADD  = $(LIBKEXIV2_LIBS) $(LIBKDCRAW_LIBS) $(LIB_KIO) \
##	                  $(LIB_SQLITE3)
#
#
#INCLUDES = $(LIBKEXIV2_CFLAGS) $(LIBKDCRAW_CFLAGS) $(all_includes) \
#	       -I$(top_srcdir)/digikam/libs/dimg \
#	       -I$(top_srcdir)/digikam/libs/dmetadata \
#	       -I$(top_srcdir)/digikam/libs/debug \
#	       -I$(top_srcdir)/digikam/sqlite
#
#
#digikaminclude_HEADERS = albumdb.h
#digikamincludedir = $(includedir)/digikam
