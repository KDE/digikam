INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/jpegutils ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/database ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(kio_digikamthumbnail_PART_SRCS
   digikamthumbnail.cpp
   )

KDE3_AUTOMOC(${kio_digikamthumbnail_PART_SRCS})

KDE3_ADD_KPART(kio_digikamthumbnail ${kio_digikamthumbnail_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamthumbnail  ${QT_AND_KDECORE_LIBS} kio jpegutils dimg )

INSTALL(TARGETS kio_digikamthumbnail DESTINATION lib/kde3)


########### next target ###############

SET(kio_digikamtags_PART_SRCS
   digikamtags.cpp
   sqlitedb.cpp
   )

KDE3_AUTOMOC(${kio_digikamtags_PART_SRCS})

KDE3_ADD_KPART(kio_digikamtags ${kio_digikamtags_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamtags  ${QT_AND_KDECORE_LIBS} kio database sqlite dimg )

INSTALL(TARGETS kio_digikamtags DESTINATION lib/kde3)


########### next target ###############

SET(kio_digikamalbums_PART_SRCS
   digikamalbums.cpp
   sqlitedb.cpp
   )

KDE3_AUTOMOC(${kio_digikamalbums_PART_SRCS})

KDE3_ADD_KPART(kio_digikamalbums ${kio_digikamalbums_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamalbums  ${QT_AND_KDECORE_LIBS} jpegutils kio dimg database sqlite )

INSTALL(TARGETS kio_digikamalbums DESTINATION lib/kde3)


########### next target ###############

SET(kio_digikamdates_PART_SRCS
   digikamdates.cpp
   sqlitedb.cpp
   )

KDE3_AUTOMOC(${kio_digikamdates_PART_SRCS})

KDE3_ADD_KPART(kio_digikamdates ${kio_digikamdates_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamdates  ${QT_AND_KDECORE_LIBS} kio database sqlite dimg )

INSTALL(TARGETS kio_digikamdates DESTINATION lib/kde3)


########### next target ###############

SET(kio_digikamsearch_PART_SRCS
   digikamsearch.cpp
   sqlitedb.cpp
   )

KDE3_AUTOMOC(${kio_digikamsearch_PART_SRCS})

KDE3_ADD_KPART(kio_digikamsearch ${kio_digikamsearch_PART_SRCS})

TARGET_LINK_LIBRARIES(kio_digikamsearch  ${QT_AND_KDECORE_LIBS} kio database sqlite dimg )

INSTALL(TARGETS kio_digikamsearch DESTINATION lib/kde3)


########### install files ###############

INSTALL_FILES( /share/services FILES  digikamthumbnail.protocol digikamtags.protocol  		        digikamalbums.protocol digikamdates.protocol  		        digikamsearch.protocol )



#original Makefile.am contents follow:

#METASOURCES = AUTO  
#
#INCLUDES = -I$(top_srcdir)/digikam/libs/jpegutils \
#	       -I$(top_srcdir)/digikam/libs/dimg \
#	       -I$(top_srcdir)/digikam/libs/dmetadata \
#	       -I$(top_srcdir)/digikam/libs/database \
#	       -I$(top_srcdir)/digikam/digikam \
#	       $(LIBKEXIV2_CFLAGS) $(LIBKDCRAW_CFLAGS) \
#	       $(all_includes)
#
#kde_module_LTLIBRARIES = kio_digikamthumbnail.la kio_digikamtags.la \
#			             kio_digikamalbums.la kio_digikamdates.la \
#			             kio_digikamsearch.la
#
#protocol_DATA = digikamthumbnail.protocol digikamtags.protocol \
#		        digikamalbums.protocol digikamdates.protocol \
#		        digikamsearch.protocol
#
#protocoldir = $(kde_servicesdir)
#
## -- digikam KIO image thumbnails generator ----------------------------------------
#
#kio_digikamthumbnail_la_SOURCES = digikamthumbnail.cpp 
#
#kio_digikamthumbnail_la_LIBADD  = $(LIB_KIO) $(LIBJPEG) $(LIBPNG)  \
#	                              $(top_builddir)/digikam/libs/jpegutils/libjpegutils.la \
#	                              $(top_builddir)/digikam/libs/dimg/libdimg.la
#
#kio_digikamthumbnail_la_LDFLAGS = -module -avoid-version $(KDE_PLUGIN) $(all_libraries)	                          
#
## -- digikam tags KIO --------------------------------------------------------------
#
#kio_digikamtags_la_SOURCES = digikamtags.cpp sqlitedb.cpp
#
#kio_digikamtags_la_LIBADD  = $(LIB_KIO)  $(LIB_SQLITE3) \
#	                     $(LIBKEXIV2_LIBS) $(LIBKDCRAW_LIBS) \
#	                     $(top_builddir)/digikam/libs/database/libdatabase.la \
#	                     $(top_builddir)/digikam/sqlite/libsqlite.la \
#	                     $(top_builddir)/digikam/libs/dimg/libdimg.la
#
#kio_digikamtags_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN) 
#
## -- digikam albums KIO ------------------------------------------------------------
#
#kio_digikamalbums_la_SOURCES = digikamalbums.cpp sqlitedb.cpp
#
#kio_digikamalbums_la_LIBADD  = $(top_builddir)/digikam/libs/jpegutils/libjpegutils.la \
#	                           $(LIB_KIO) $(LIB_SQLITE3) \
#	                           $(top_builddir)/digikam/libs/dimg/libdimg.la \
#	                           $(top_builddir)/digikam/libs/database/libdatabase.la \
#	                           $(top_builddir)/digikam/sqlite/libsqlite.la
#
#kio_digikamalbums_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN) 
#
## -- digikam dates KIO -------------------------------------------------------------
#
#kio_digikamdates_la_SOURCES = digikamdates.cpp sqlitedb.cpp
#
#kio_digikamdates_la_LIBADD  = $(LIB_KIO) $(LIB_SQLITE3)\
#	                      $(LIBKEXIV2_LIBS) $(LIBKDCRAW_LIBS) \
#	                      $(top_builddir)/digikam/libs/database/libdatabase.la \
#	                      $(top_builddir)/digikam/sqlite/libsqlite.la \
#	                      $(top_builddir)/digikam/libs/dimg/libdimg.la
#
#kio_digikamdates_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN) 
#
## -- digikam search KIO ------------------------------------------------------------
#
#kio_digikamsearch_la_SOURCES = digikamsearch.cpp sqlitedb.cpp
#
#kio_digikamsearch_la_LIBADD  = $(LIB_KIO) $(LIB_SQLITE3)\
#	                       $(LIBKEXIV2_LIBS) $(LIBKDCRAW_LIBS) \
#	                       $(top_builddir)/digikam/libs/database/libdatabase.la \
#	                       $(top_builddir)/digikam/sqlite/libsqlite.la \
#	                       $(top_builddir)/digikam/libs/dimg/libdimg.la
#
#kio_digikamsearch_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN) 
