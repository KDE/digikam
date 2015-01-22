set(libpgfutils_SRCS pgfutils.cpp)

set(libthreadimageio_SRCS
    filereadwritelock.cpp
    loadsavethread.cpp
    managedloadsavethread.cpp
    sharedloadsavethread.cpp
    loadingdescription.cpp
    loadingcache.cpp
    loadingcacheinterface.cpp
    loadsavetask.cpp
    previewloadthread.cpp
    previewtask.cpp
    previewsettings.cpp
    thumbnailbasic.cpp
    thumbnailcreator.cpp
    thumbnailloadthread.cpp
    thumbnailtask.cpp
    thumbnailsize.cpp
)

add_library(threadimageio STATIC ${libthreadimageio_SRCS} ${libpgfutils_SRCS})

target_include_directories(threadimageio
                              PUBLIC
                              $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(threadimageio
                      digikamdatabase
                      dthread
                      
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets # QMessageBox in ThumbnailLoadThread..
                      Qt5::Sql
                      Qt5::DBus
                      
                      KF5::KIOWidgets # for previewjob
                      KF5::KDELibs4Support # for KSharedPtr
)