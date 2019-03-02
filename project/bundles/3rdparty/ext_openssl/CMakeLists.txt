# Script to build OpenSSL for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(EXTPREFIX_openssl "${EXTPREFIX}")

ExternalProject_Add(ext_openssl
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/openssl/openssl/archive/OpenSSL_1_1_1b.tar.gz
    URL_MD5 bfa09d75407ea860f22ddc0312e1cc49

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_openssl}
               -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE}
               ${GLOBAL_PROFILE}

    UPDATE_COMMAND ""
    ALWAYS 0
)
