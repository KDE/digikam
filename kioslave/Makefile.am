METASOURCES = AUTO  

if with_included_sqlite3
    LIB_SQLITE3_LOCAL = $(top_builddir)/digikam/libs/sqlite3/libsqlite3.la
    SQLITE3_INCLUDES = -I$(top_srcdir)/digikam/libs/sqlite3
endif

INCLUDES = -I$(top_srcdir)/digikam/libs/jpegutils \
	       -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/libs/dmetadata \
	       -I$(top_srcdir)/digikam/digikam \
	       $(SQLITE3_INCLUDES) \
	       $(LIBKEXIV2_CFLAGS) $(LIBKDCRAW_CFLAGS) \
	       $(all_includes)

kde_module_LTLIBRARIES = kio_digikamthumbnail.la kio_digikamtags.la \
			             kio_digikamalbums.la kio_digikamdates.la \
			             kio_digikamsearch.la

protocol_DATA = digikamthumbnail.protocol digikamtags.protocol \
		        digikamalbums.protocol digikamdates.protocol \
		        digikamsearch.protocol

protocoldir = $(kde_servicesdir)

# NOTE: if local libsqlite3 is used LIB_SQLITE3 is null.
#       if shared libsqlite3 is used LIB_SQLITE3_LOCAL is null.

# -- digikam KIO image thumbnails generator ----------------------------------------

kio_digikamthumbnail_la_SOURCES = digikamthumbnail.cpp 

kio_digikamthumbnail_la_LIBADD  = $(LIB_KIO) $(LIBJPEG) $(LIBPNG) \
	                              $(top_builddir)/digikam/libs/jpegutils/libjpegutils.la \
	                              $(top_builddir)/digikam/libs/dimg/libdimg.la

kio_digikamthumbnail_la_LDFLAGS = -module -avoid-version $(KDE_PLUGIN) $(all_libraries)	                          

# -- digikam tags KIO --------------------------------------------------------------

kio_digikamtags_la_SOURCES = digikamtags.cpp sqlitedb.cpp

kio_digikamtags_la_LIBADD  = $(LIB_KIO) $(LIB_SQLITE3) $(LIB_SQLITE3_LOCAL) 

kio_digikamtags_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN) 

# -- digikam albums KIO ------------------------------------------------------------

kio_digikamalbums_la_SOURCES = digikamalbums.cpp sqlitedb.cpp

kio_digikamalbums_la_LIBADD  = $(top_builddir)/digikam/libs/jpegutils/libjpegutils.la \
	                           $(top_builddir)/digikam/libs/dimg/libdimg.la \
	                           $(LIB_KIO) $(LIB_SQLITE3) $(LIB_SQLITE3_LOCAL) 


kio_digikamalbums_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN) 

# -- digikam dates KIO -------------------------------------------------------------

kio_digikamdates_la_SOURCES = digikamdates.cpp sqlitedb.cpp

kio_digikamdates_la_LIBADD  = $(LIB_KIO) $(LIB_SQLITE3) $(LIB_SQLITE3_LOCAL) 

kio_digikamdates_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN) 

# -- digikam search KIO ------------------------------------------------------------

kio_digikamsearch_la_SOURCES = digikamsearch.cpp sqlitedb.cpp

kio_digikamsearch_la_LIBADD  = $(LIB_KIO) $(LIB_SQLITE3) $(LIB_SQLITE3_LOCAL) 

kio_digikamsearch_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN) 
