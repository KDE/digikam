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
    URL https://gitlab.com/libtiff/libtiff/-/archive/Release-v4-0-9/libtiff-Release-v4-0-9.tar.bz2
    URL_MD5 107e5cbe8204df361ac117841dfec2fb

    CONFIGURE_COMMAND <SOURCE_DIR>/configure -prefix ${EXTPREFIX_tiff}
    UPDATE_COMMAND ""
    ALWAYS 0
)

