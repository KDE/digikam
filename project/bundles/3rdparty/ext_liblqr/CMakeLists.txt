# Script to build liblqr for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(EXTPREFIX_liblqr "${EXTPREFIX}")

ExternalProject_Add(ext_liblqr
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/carlobaldassi/liblqr/archive/v0.4.2.tar.gz
    URL_MD5 f7b4ecddd8a038f057e236347fd7767c

    INSTALL_DIR ${EXTPREFIX_liblqr}
    CONFIGURE_COMMAND <SOURCE_DIR>/configure -prefix ${EXTPREFIX_liblqr}

    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    ALWAYS 0
)
