
include_directories( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata
                     ${CMAKE_SOURCE_DIR}/digikam/libs/jpegutils
                     ${CMAKE_SOURCE_DIR}/digikam/digikam
                     ${KDE4_INCLUDE_DIR}
                     ${QT_INCLUDES}
                   )

SET(libthreadimageio_STAT_SRCS loadingcache.cpp )

kde4_automoc(${libthreadimageio_STAT_SRCS})

#loadsavethread.cpp
#                               managedloadsavethread.cpp
#                               sharedloadsavethread.cpp
#                               previewloadthread.cpp
#                               loadingdescription.cpp
#                               loadsavetask.cpp
#                               previewtask.cpp
#                               loadingcacheinterface.cpp

#TARGET_LINK_LIBRARIES(libdimg
#                      libdmetadata
#                      libdimgloaders
#                      ${KDCRAW_LIBRARIES}
#                     )

KDE4_ADD_LIBRARY(libthreadimageio STATIC ${libthreadimageio_STAT_SRCS})

#INSTALL( FILES
#         DESTINATION ${INCLUDE_INSTALL_DIR}/digikam )




########### next target ###############


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
