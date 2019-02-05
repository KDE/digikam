#
# Copyright (c) 2010-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

kde_enable_exceptions()

add_definitions(${EXIV2_DEFINITIONS})

if( WIN32 )
    add_definitions( -DEXV_HAVE_DLL )
endif()

set(libdmetadata_SRCS
    engine/metaengine.cpp
    engine/metaengine_p.cpp
    engine/metaengine_data.cpp
    engine/metaengine_data_p.cpp
    engine/metaengine_exif.cpp
    engine/metaengine_fileio.cpp
    engine/metaengine_item.cpp
    engine/metaengine_comments.cpp
    engine/metaengine_iptc.cpp
    engine/metaengine_gps.cpp
    engine/metaengine_xmp.cpp
    engine/metaengine_previews.cpp
    engine/metaengine_rotation.cpp
    engine/metaenginesettings.cpp
    engine/metaenginesettingscontainer.cpp
    dmetadata/dmetadata.cpp
    dmetadata/dmetadata_video.cpp
    dmetadata/dmetadata_fileio.cpp
    dmetadata/dmetadata_comments.cpp
    dmetadata/dmetadata_template.cpp
    dmetadata/dmetadata_exif.cpp
    dmetadata/dmetadata_iptc.cpp
    dmetadata/dmetadata_labels.cpp
    dmetadata/dmetadata_faces.cpp
    dmetadata/dmetadata_history.cpp
    dmetadata/dmetadata_photo.cpp
    dmetadata/dmetadata_tags.cpp
    dmetadata/dmetadata_generic.cpp
    dmetadata/dmetadata_xmp.cpp
    dmetadata/dmetadatasettings.cpp
    dmetadata/dmetadatasettingscontainer.cpp
    dmetadata/geodetictools.cpp
    containers/metadatainfo.cpp
    containers/photoinfocontainer.cpp
    containers/videoinfocontainer.cpp
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
