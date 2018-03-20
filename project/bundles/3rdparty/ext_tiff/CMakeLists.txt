# Script to build libgphoto for digiKam bundle.
#
# Copyright (c) 2015-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(EXTPREFIX_tiff "${EXTPREFIX}" )

ExternalProject_Add(ext_tiff
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL ftp://download.osgeo.org/libtiff/tiff-4.0.9.tar.gz
    URL_MD5 54bad211279cc93eb4fca31ba9bfdc79

    CONFIGURE_COMMAND <SOURCE_DIR>/configure -prefix ${EXTPREFIX_tiff}
    UPDATE_COMMAND ""
    ALWAYS 0
)

