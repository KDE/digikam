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

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/heif3.4.0-centos6.patch

    CONFIGURE_COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_heif}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
               -DHEIF_GCC_ALLOCATOR_FIX=1
               ${GLOBAL_PROFILE}
               <SOURCE_DIR>/srcs

    UPDATE_COMMAND ""
    ALWAYS 0
)
