# Script to build libheif for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(PREFIX_ext_heif "${EXTPREFIX}")

ExternalProject_Add(de265
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/strukturag/libde265/archive/v1.0.3.tar.gz
    URL_MD5 189b4deb376ff879f8cbb2b7ffc24f97

    INSTALL_DIR ${EXTPREFIX}

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
               ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    ALWAYS 0
)

ExternalProject_Add(ext_heif
    DEPENDS de265

    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/strukturag/libheif/archive/v1.4.0.tar.gz
    URL_MD5 05dea500c105289c74270791cb5f8942

    INSTALL_DIR ${PREFIX_ext_heif}

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_heif}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
               ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    ALWAYS 0
)
