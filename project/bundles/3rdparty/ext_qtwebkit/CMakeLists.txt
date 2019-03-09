# Script to build QtWebkit for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(EXTPREFIX_qtwebkit "${EXTPREFIX}")

ExternalProject_Add(ext_qtwebkit
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.qt.io/snapshots/ci/qtwebkit/5.212/latest/src/submodules/qtwebkit-everywhere-src-5.212.tar.xz
    URL_MD5 48b9afcf920ff8077a7b05316851c38c
    INSTALL_DIR ${EXTPREFIX_qtwebkit}

    CONFIGURE_COMMAND qmake PREFIX=${EXTPREFIX_qtwebkit}
    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    ALWAYS 0
)
