#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

kde_enable_exceptions()

add_definitions(${EXIV2_DEFINITIONS})

if( WIN32 )
    add_definitions( -DEXV_HAVE_DLL )
endif()

set(libdmetadata_SRCS
    metaengine.cpp
    metaengine_p.cpp
    metaengine_data.cpp
    metaengine_image.cpp
    metaengine_comments.cpp
    metaengine_exif.cpp
    metaengine_iptc.cpp
    metaengine_gps.cpp
    metaengine_xmp.cpp
    metaengine_previews.cpp
    metaengine_rotation.cpp
    dmetadata.cpp
    dmetadata_ffmpeg.cpp
    dmetadatasettings.cpp
    dmetadatasettingscontainer.cpp
    metadatasettings.cpp
    metadatasettingscontainer.cpp
    metadatainfo.cpp
    infocontainer.cpp
    geodetictools.cpp
    template.cpp
    captionvalues.cpp
)

include_directories(
    ${EXIV2_INCLUDE_DIR}
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(dmetadata_src OBJECT ${libdmetadata_SRCS})
