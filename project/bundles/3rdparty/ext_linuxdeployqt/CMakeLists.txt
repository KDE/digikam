# Script to build linuxdeployqt for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(EXTPREFIX_linuxdeployqt "${EXTPREFIX}")

ExternalProject_Add(ext_linuxdeployqt
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}

    GIT_REPOSITORY https://github.com/probonopd/linuxdeployqt.git
    GIT_TAG 6

    INSTALL_DIR ${EXTPREFIX_linuxdeployqt}
    CONFIGURE_COMMAND qmake PREFIX=${EXTPREFIX_linuxdeployqt}

    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    ALWAYS 0
)
