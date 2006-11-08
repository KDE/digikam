METASOURCES = AUTO

noinst_LTLIBRARIES = libthreadimageio.la

libthreadimageio_la_SOURCES = loadsavethread.cpp \
                              managedloadsavethread.cpp \
                              sharedloadsavethread.cpp \
                              loadsavetask.cpp \
                              loadingcache.cpp \
                              loadingcacheinterface.cpp

libthreadimageio_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
	       -I$(top_srcdir)/digikam/libs/dimg/loaders \
	       -I$(top_srcdir)/digikam/digikam \
	       $(all_includes)
