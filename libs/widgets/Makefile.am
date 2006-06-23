SUBDIRS = metadata imageplugins common iccprofiles

noinst_LTLIBRARIES = libwidgets.la

libwidgets_la_SOURCES =

libwidgets_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

libwidgets_la_LIBADD = $(top_builddir)/digikam/libs/widgets/metadata/libmetadatawidgets.la \
	               $(top_builddir)/digikam/libs/widgets/iccprofiles/libiccprofileswidgets.la \
	               $(top_builddir)/digikam/libs/widgets/imageplugins/libimagepluginswidgets.la \
	               $(top_builddir)/digikam/libs/widgets/common/libcommonwidgets.la 
