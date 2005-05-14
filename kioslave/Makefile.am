INCLUDES    = $(IMLIB2_CFLAGS) $(LIBKEXIF_CFLAGS) -I$(top_srcdir)/digikam/digikam -I$(top_srcdir)/digikam/sqlite $(all_includes) 
METASOURCES = AUTO  
KDE_CFLAGS =  -w

kde_module_LTLIBRARIES = kio_digikamthumbnail.la kio_digikamtags.la \
			 kio_digikamio.la

kio_digikamthumbnail_la_SOURCES = digikamthumbnail.cpp parse.c
kio_digikamthumbnail_la_LIBADD = $(LIB_KIO) $(LIBJPEG) $(LIBPNG) 
kio_digikamthumbnail_la_LDFLAGS = -module -avoid-version $(all_libraries) \
	$(KDE_PLUGIN) $(LIB_IMLIB2) $(LIBKEXIF_LIBS)

kio_digikamtags_la_SOURCES  = digikamtags.cpp
kio_digikamtags_la_LIBADD = $(LIB_KIO) $(top_builddir)/digikam/sqlite/libsqlite.la
kio_digikamtags_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN) 

kio_digikamio_la_SOURCES = digikamio.cpp
kio_digikamio_la_LIBADD = $(LIB_KIO) $(top_builddir)/digikam/sqlite/libsqlite.la
kio_digikamio_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN) 

protocol_DATA = digikamthumbnail.protocol digikamtags.protocol \
		digikamio.protocol
protocoldir = $(kde_servicesdir)
