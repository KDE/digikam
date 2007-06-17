
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/loaders
                     ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata
                     ${CMAKE_SOURCE_DIR}/digikam/libs/jpegutils
                   )

SET(libthreadimageio_STAT_SRCS loadsavethread.cpp managedloadsavethread.cpp sharedloadsavethread.cpp
                               loadingdescription.cpp loadingcache.cpp loadingcacheinterface.cpp
                               loadsavetask.cpp previewloadthread.cpp previewtask.cpp
   )

KDE4_AUTOMOC(${libthreadimageio_STAT_SRCS})

KDE4_ADD_LIBRARY(libthreadimageio STATIC ${libthreadimageio_STAT_SRCS})

