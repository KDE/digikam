
include_directories( ${CMAKE_SOURCE_DIR}/digikam/libs/jpegutils ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/database ${CMAKE_SOURCE_DIR}/digikam/digikam ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(kio_digikamthumbnail_PART_SRCS digikamthumbnail.cpp )

kde4_automoc(${kio_digikamthumbnail_PART_SRCS})

kde4_add_plugin(kio_digikamthumbnail ${kio_digikamthumbnail_PART_SRCS})

target_link_libraries(kio_digikamthumbnail  ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} jpegutils dimg )

install(TARGETS kio_digikamthumbnail DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(kio_digikamtags_PART_SRCS digikamtags.cpp sqlitedb.cpp )

kde4_automoc(${kio_digikamtags_PART_SRCS})

kde4_add_plugin(kio_digikamtags ${kio_digikamtags_PART_SRCS})

target_link_libraries(kio_digikamtags  ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} database sqlite dimg )

install(TARGETS kio_digikamtags DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(kio_digikamalbums_PART_SRCS digikamalbums.cpp sqlitedb.cpp )

kde4_automoc(${kio_digikamalbums_PART_SRCS})

kde4_add_plugin(kio_digikamalbums ${kio_digikamalbums_PART_SRCS})

target_link_libraries(kio_digikamalbums  ${KDE4_KDECORE_LIBS} jpegutils ${KDE4_KIO_LIBS} dimg database sqlite )

install(TARGETS kio_digikamalbums DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(kio_digikamdates_PART_SRCS digikamdates.cpp sqlitedb.cpp )

kde4_automoc(${kio_digikamdates_PART_SRCS})

kde4_add_plugin(kio_digikamdates ${kio_digikamdates_PART_SRCS})

target_link_libraries(kio_digikamdates  ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} database sqlite dimg )

install(TARGETS kio_digikamdates DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(kio_digikamsearch_PART_SRCS digikamsearch.cpp sqlitedb.cpp )

kde4_automoc(${kio_digikamsearch_PART_SRCS})

kde4_add_plugin(kio_digikamsearch ${kio_digikamsearch_PART_SRCS})

target_link_libraries(kio_digikamsearch  ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} database sqlite dimg )

install(TARGETS kio_digikamsearch DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  digikamthumbnail.protocol digikamtags.protocol  		        digikamalbums.protocol digikamdates.protocol  		        digikamsearch.protocol DESTINATION ${SERVICES_INSTALL_DIR} )



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
