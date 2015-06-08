#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libdmetadata_SRCS
    metadatasettings.cpp
    metadatasettingscontainer.cpp
    metadatainfo.cpp
    infocontainer.cpp
    dmetadata.cpp
    geodetictools.cpp
    template.cpp
    captionvalues.cpp
)

add_library(dmetadata_src OBJECT ${libdmetadata_SRCS})
add_library(dmetadata STATIC $<TARGET_OBJECTS:dmetadata_src>)

add_dependencies(dmetadata digikam-gitversion)

target_link_libraries(dmetadata
                      PRIVATE
                      dimg

                      Qt5::Core
)
