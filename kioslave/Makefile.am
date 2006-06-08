METASOURCES = AUTO  

INCLUDES = -I$(top_srcdir)/digikam/libs/jpegutils \
           -I$(top_srcdir)/digikam/digikam \
	   $(IMLIB2_CFLAGS) $(LIBKEXIF_CFLAGS) $(all_includes) 

kde_module_LTLIBRARIES = kio_digikamthumbnail.la kio_digikamtags.la \
			 kio_digikamalbums.la kio_digikamdates.la \
			 kio_digikamsearch.la

protocol_DATA = digikamthumbnail.protocol digikamtags.protocol \
		digikamalbums.protocol digikamdates.protocol \
		digikamsearch.protocol

protocoldir = $(kde_servicesdir)

KDE_CFLAGS = -w

# -- digikam KIO image thumbnails generator ----------------------------------------

kio_digikamthumbnail_la_SOURCES = digikamthumbnail.cpp

kio_digikamthumbnail_la_LIBADD = $(LIB_KIO) $(LIBJPEG) $(LIBPNG) 

kio_digikamthumbnail_la_LDFLAGS = -module -avoid-version $(all_libraries) \
	                          $(KDE_PLUGIN) $(LIB_IMLIB2) $(LIBKEXIF_LIBS)

# -- digikam tags KIO --------------------------------------------------------------

kio_digikamtags_la_SOURCES  = digikamtags.cpp sqlitedb.cpp

kio_digikamtags_la_LIBADD = $(LIB_KIO) $(LIB_SQLITE3)

kio_digikamtags_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN) 

# -- digikam albums KIO ------------------------------------------------------------

kio_digikamalbums_la_SOURCES  = digikamalbums.cpp sqlitedb.cpp

kio_digikamalbums_la_LIBADD = $(top_builddir)/digikam/libs/jpegutils/libjpegutils.la \
			      $(LIB_KIO) $(LIB_SQLITE3)

kio_digikamalbums_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN) 

# -- digikam dates KIO -------------------------------------------------------------

kio_digikamdates_la_SOURCES  = digikamdates.cpp sqlitedb.cpp

kio_digikamdates_la_LIBADD = $(LIB_KIO) $(LIB_SQLITE3)

kio_digikamdates_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN) 

# -- digikam search KIO ------------------------------------------------------------

kio_digikamsearch_la_SOURCES  = digikamsearch.cpp sqlitedb.cpp

kio_digikamsearch_la_LIBADD = $(LIB_KIO) $(LIB_SQLITE3)

kio_digikamsearch_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN) 
