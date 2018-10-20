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
    engine/metaengine.cpp
    engine/metaengine_p.cpp
    engine/metaengine_data.cpp
    engine/metaengine_image.cpp
    engine/metaengine_comments.cpp
    engine/metaengine_exif.cpp
    engine/metaengine_iptc.cpp
    engine/metaengine_gps.cpp
    engine/metaengine_xmp.cpp
    engine/metaengine_previews.cpp
    engine/metaengine_rotation.cpp
    engine/metaenginesettings.cpp
    engine/metaenginesettingscontainer.cpp
    dmetadata/dmetadata.cpp
    dmetadata/dmetadata_ffmpeg.cpp
    dmetadata/dmetadata_libraw.cpp
    dmetadata/dmetadata_comments.cpp
    dmetadata/dmetadata_template.cpp
    dmetadata/dmetadata_iptc.cpp
    dmetadata/dmetadata_labels.cpp
    dmetadata/dmetadatasettings.cpp
    dmetadata/dmetadatasettingscontainer.cpp
    containers/metadatainfo.cpp
    containers/infocontainer.cpp
    containers/template.cpp
    containers/captionvalues.cpp
)

include_directories(
    ${EXIV2_INCLUDE_DIR}
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(dmetadata_src OBJECT ${libdmetadata_SRCS})
