# Script to build libpng for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(PREFIX_ext_png "${EXTPREFIX}")

ExternalProject_Add(ext_png
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://prdownloads.sourceforge.net/libpng/libpng-1.6.36.tar.xz
    URL_MD5 df2be2d29c40937fe1f5349b16bc2826

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_png}
    UPDATE_COMMAND ""
    ALWAYS 0
)
