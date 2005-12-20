INCLUDES    = -I$(top_srcdir)/digikam/libs/jpegutils \
	          -I$(top_srcdir)/digikam/libs/dimg \
	          -I$(top_srcdir)/digikam/libs/dcraw \
	          -I$(top_srcdir)/digikam/digikam \
	          $(LIBKEXIF_CFLAGS) $(all_includes)

METASOURCES = AUTO  
KDE_CFLAGS  = -w

kde_module_LTLIBRARIES = kio_digikamthumbnail.la kio_digikamtags.la \
			 kio_digikamalbums.la kio_digikamdates.la \
			 kio_digikamsearch.la

kio_digikamthumbnail_la_SOURCES = digikamthumbnail.cpp 

kio_digikamthumbnail_la_LIBADD  = $(LIB_KIO) $(LIBJPEG) $(LIBPNG) \
	                          $(top_builddir)/digikam/libs/dimg/libdimg.la

kio_digikamthumbnail_la_LDFLAGS = -module -avoid-version $(all_libraries) \
	                          $(KDE_PLUGIN) $(LIBKEXIF_LIBS)

kio_digikamtags_la_SOURCES = digikamtags.cpp sqlitedb.cpp
kio_digikamtags_la_LIBADD  = $(LIB_KIO)  $(LIB_SQLITE3)
kio_digikamtags_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN) 

kio_digikamalbums_la_SOURCES = digikamalbums.cpp sqlitedb.cpp
kio_digikamalbums_la_LIBADD  = $(top_builddir)/digikam/libs/jpegutils/libjpegutils.la \
			      $(LIB_KIO) $(LIB_SQLITE3)
kio_digikamalbums_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN) 

kio_digikamdates_la_SOURCES = digikamdates.cpp sqlitedb.cpp
kio_digikamdates_la_LIBADD  = $(LIB_KIO)  $(LIB_SQLITE3)
kio_digikamdates_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN) 

kio_digikamsearch_la_SOURCES = digikamsearch.cpp sqlitedb.cpp
kio_digikamsearch_la_LIBADD  = $(LIB_KIO) $(LIB_SQLITE3)
kio_digikamsearch_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN) 

protocol_DATA = digikamthumbnail.protocol digikamtags.protocol \
		digikamalbums.protocol digikamdates.protocol \
		digikamsearch.protocol

protocoldir = $(kde_servicesdir)
