INCLUDES    = $(all_includes)

kde_module_LTLIBRARIES = kio_digikamthumbnail.la

kio_digikamthumbnail_la_SOURCES = digikamthumbnail.cpp
kio_digikamthumbnail_la_LIBADD = $(LIB_KIO) -ljpeg
kio_digikamthumbnail_la_LDFLAGS = -module -avoid-version $(all_libraries) $(KDE_PLUGIN)

protocol_DATA = digikamthumbnail.protocol
protocoldir = $(kde_servicesdir)
