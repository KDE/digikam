# Script to build libgphoto for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(EXTPREFIX_libgphoto2 "${EXTPREFIX}")

ExternalProject_Add(ext_libgphoto2
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL http://downloads.sourceforge.net/project/gphoto/libgphoto/2.5.20/libgphoto2-2.5.20.tar.bz2
    URL_MD5 ba4743fa6e68406323095cc165306a21

    INSTALL_DIR ${EXTPREFIX_libgphoto2}
    CONFIGURE_COMMAND <SOURCE_DIR>/configure -prefix ${EXTPREFIX_libgphoto2}

    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    ALWAYS 0
)
