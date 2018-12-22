# Script to build sane backends for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(EXTPREFIX_sane "${EXTPREFIX}")

ExternalProject_Add(ext_sane
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://gitlab.com/sane-project/backends/-/archive/RELEASE_1_0_27/backends-RELEASE_1_0_27.tar.bz2
    URL_MD5 25a5d5a9f105df116ed09ad9c1745cfd

    INSTALL_DIR ${EXTPREFIX_sane}
    CONFIGURE_COMMAND <SOURCE_DIR>/configure -prefix ${EXTPREFIX_sane}
    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    ALWAYS 0
)
