INCLUDES    = -I$(top_srcdir)/digikam/sqlite $(all_includes) 

kde_module_LTLIBRARIES = kio_digikamthumbnail.la kio_digikamtags.la

kio_digikamthumbnail_la_SOURCES = digikamthumbnail.cpp
kio_digikamthumbnail_la_LIBADD = $(LIB_KIO) -ljpeg
kio_digikamthumbnail_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN) $(LIB_IMLIB2)

kio_digikamtags_la_SOURCES  = digikamtags.cpp
kio_digikamtags_la_LIBADD = $(LIB_KIO) $(top_builddir)/digikam/sqlite/libsqlite.la
kio_digikamtags_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN) 

protocol_DATA = digikamthumbnail.protocol digikamtags.protocol
protocoldir = $(kde_servicesdir)
