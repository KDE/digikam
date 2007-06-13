INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/loaders ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/jpegutils ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(threadimageio_STAT_SRCS
   loadsavethread.cpp
   managedloadsavethread.cpp
   sharedloadsavethread.cpp
   previewloadthread.cpp
   loadingdescription.cpp
   loadsavetask.cpp
   previewtask.cpp
   loadingcache.cpp
   loadingcacheinterface.cpp
   )

KDE3_AUTOMOC(${threadimageio_STAT_SRCS})

ADD_LIBRARY(threadimageio STATIC ${threadimageio_STAT_SRCS})


########### install files ###############




#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libthreadimageio.la
#
#libthreadimageio_la_SOURCES = loadsavethread.cpp \
#                              managedloadsavethread.cpp \
#                              sharedloadsavethread.cpp \
#                              previewloadthread.cpp \
#                              loadingdescription.cpp \
#                              loadsavetask.cpp \
#                              previewtask.cpp \
#                              loadingcache.cpp \
#                              loadingcacheinterface.cpp
#
#libthreadimageio_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#INCLUDES = -I$(top_srcdir)/digikam/libs/dimg \
#	       -I$(top_srcdir)/digikam/libs/dimg/loaders \
#	       -I$(top_srcdir)/digikam/libs/dmetadata \
#	       $(LIBKEXIV2_CFLAGS) \
#	       -I$(top_srcdir)/digikam/libs/jpegutils \
#	       -I$(top_srcdir)/digikam/digikam \
#	       $(LIBKDCRAW_CFLAGS) $(all_includes)
