# Script to build libicu for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(EXTPREFIX_iconv "${EXTPREFIX}")

ExternalProject_Add(ext_iconv
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.15.tar.gz
    URL_MD5 ace8b5f2db42f7b3b3057585e80d9808

    INSTALL_DIR ${EXTPREFIX_iconv}

    CONFIGURE_COMMAND ./configure --prefix ${EXTPREFIX_iconv}

    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    ALWAYS 0
)
