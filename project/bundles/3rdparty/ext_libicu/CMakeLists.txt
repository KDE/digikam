# Script to build libicu for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(EXTPREFIX_libicu "${EXTPREFIX}")

ExternalProject_Add(ext_libicu
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/unicode-org/icu/releases/download/release-63-1/icu4c-63_1-src.tgz
    URL_MD5 9e40f6055294284df958200e308bce50

    INSTALL_DIR ${EXTPREFIX_libicu}
    CONFIGURE_COMMAND <SOURCE_DIR>/configure -prefix ${EXTPREFIX_libicu}

    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    ALWAYS 0
)
