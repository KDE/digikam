# Script to build libfontconfig for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(PREFIX_ext_fontconfig "${EXTPREFIX}")

ExternalProject_Add(ext_fontconfig
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://www.freedesktop.org/software/fontconfig/release/fontconfig-2.13.1.tar.bz2
    URL_MD5 36cdea1058ef13cbbfdabe6cb019dc1c

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_fontconfig} --disable-docs

    UPDATE_COMMAND ""
    ALWAYS 0
)
