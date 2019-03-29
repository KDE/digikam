# Script to build libass for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(EXTPREFIX_libass "${EXTPREFIX}")

ExternalProject_Add(ext_libass
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://github.com/libass/libass/releases/download/0.14.0/libass-0.14.0.tar.xz
    URL_MD5 5b8c23340654587b8a472cb74ee9366b

    INSTALL_DIR ${EXTPREFIX_libass}
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix ${EXTPREFIX_libass} --disable-require-system-font-provider

    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    ALWAYS 0
)
