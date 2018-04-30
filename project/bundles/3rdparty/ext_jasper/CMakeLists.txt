# Script to build jasperlib for digiKam bundle.
#
# Copyright (c) 2015-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(PREFIX_ext_jasper "${EXTPREFIX}")

ExternalProject_Add(ext_jasper
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/mdadams/jasper/archive/version-2.0.14.tar.gz
    URL_MD5 9ef0e943d5b2c364ae84974a753841b8

    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=${PREFIX_ext_jasper} ${GLOBAL_AUTOMAKE_PROFILE} --disable-opengl
    UPDATE_COMMAND ""
    ALWAYS 0
)
