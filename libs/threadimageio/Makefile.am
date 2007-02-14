METASOURCES = AUTO

noinst_LTLIBRARIES = libthreadimageio.la

libthreadimageio_la_SOURCES = loadsavethread.cpp \
                              managedloadsavethread.cpp \
                              sharedloadsavethread.cpp \
                              previewloadthread.cpp \
                              loadingdescription.cpp \
                              loadsavetask.cpp \
                              previewtask.cpp \
                              loadingcache.cpp \
                              loadingcacheinterface.cpp

libthreadimageio_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/libs/dimg/loaders \
	       -I$(top_srcdir)/digikam/libs/dmetadata \
	       -I$(top_srcdir)/digikam/libs/dcraw \
	       -I$(top_srcdir)/digikam/libs/jpegutils \
	       -I$(top_srcdir)/digikam/digikam \
	       $(all_includes)
