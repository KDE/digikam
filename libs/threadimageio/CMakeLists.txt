# TODO move pgfutils to own dir?

# Enable C++ Exceptions support, required by pgfutils
kde_source_files_enable_exceptions(pgfutils.cpp)

message(INFO " move pgfutils to its own subdir from threadimageio?")
include_directories(${PGF_INCLUDE_DIRS})

set(libpgfutils_SRCS pgfutils.cpp)

add_library(pgfutils ${libpgfutils_SRCS})

target_include_directories(pgfutils PUBLIC ${PGF_INCLUDE_DIRS})

target_link_libraries(pgfutils
                         Qt5::Gui
                         ${PGF_LIBRARIES}
)


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

target_link_libraries(threadimageio
                      digikamdatabase
                      dthread
                      pgfutils
                      
                      Qt5::Core
                      Qt5::Gui
                      Qt5::Widgets # QMessageBox in ThumbnailLoadThread..
                      Qt5::Sql
                      Qt5::DBus
                      
                      KF5::KIOWidgets # for previewjob
                      KF5::KDELibs4Support # for KSharedPtr
)