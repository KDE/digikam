# Script to build libfreetype for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(PREFIX_ext_freetype "${EXTPREFIX}")

ExternalProject_Add(ext_freetype
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://downloads.sourceforge.net/project/freetype/freetype2/2.6.5/freetype-2.6.5.tar.bz2
    URL_MD5 6a386964e18ba28cb93370e57a19031b

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_freetype}

    UPDATE_COMMAND ""
    ALWAYS 0
)
