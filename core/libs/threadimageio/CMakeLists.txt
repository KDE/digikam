#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(libthreadimageio_SRCS
    engine/dfileoperations.cpp
    engine/filereadwritelock.cpp
    engine/managedloadsavethread.cpp
    engine/sharedloadsavethread.cpp
    preview/previewloadthread.cpp
    preview/previewtask.cpp
    preview/previewsettings.cpp
    thumb/thumbnailbasic.cpp
    thumb/thumbnailcreator.cpp
    thumb/thumbnailloadthread.cpp
    thumb/thumbnailtask.cpp
    thumb/thumbnailsize.cpp
    fileio/loadsavethread.cpp
    fileio/loadingdescription.cpp
    fileio/loadingcache.cpp
    fileio/loadingcacheinterface.cpp
    fileio/loadsavetask.cpp
)

if(ENABLE_MEDIAPLAYER)

    set(libthreadimageio_SRCS
        ${libthreadimageio_SRCS}
        video/videostripfilter.cpp
        video/videodecoder.cpp
        video/videodecoder_p.cpp
        video/videothumbwriter.cpp
        video/videothumbnailer.cpp
        video/videothumbnailerjob.cpp
       )
endif()

include_directories(
    $<TARGET_PROPERTY:Qt5::Sql,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::Service,INTERFACE_INCLUDE_DIRECTORIES>
)

if(ENABLE_DBUS)
    include_directories($<TARGET_PROPERTY:Qt5::DBus,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

add_library(threadimageio_src OBJECT ${libthreadimageio_SRCS})
