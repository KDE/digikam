# Script to build Boost for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(PREFIX_ext_boost "${EXTPREFIX}")

ExternalProject_Add( ext_boost
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/boostorg/boost/archive/boost-1.70.0.tar.gz
    URL_MD5 edf77d2ac1a1b46275b013d9a1e6ddbf
    CONFIGURE_COMMAND <SOURCE_DIR>/bootstrap.sh --prefix=${PREFIX_ext_boost} --with-libraries=system
    BUILD_COMMAND <SOURCE_DIR>/b2 install
    INSTALL_COMMAND ""
    INSTALL_DIR ${PREFIX_ext_boost}

    UPDATE_COMMAND ""
    ALWAYS 0
    BUILD_IN_SOURCE 1
)
