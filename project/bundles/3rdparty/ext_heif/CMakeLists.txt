# Script to build libheif for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(PREFIX_ext_heif "${EXTPREFIX}")

ExternalProject_Add(ext_heif
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/nokiatech/heif/archive/v3.4.0.tar.gz
    URL_MD5 177d34823bb4ed3ac735ca0931782ae6

    INSTALL_DIR ${PREFIX_ext_heif}

    SOURCE_SUBDIR srcs

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_heif}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
               ${GLOBAL_PROFILE}


    UPDATE_COMMAND ""
    ALWAYS 0
)
