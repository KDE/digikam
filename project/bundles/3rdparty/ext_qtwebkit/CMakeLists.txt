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

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/qtwebkit-fixdeps.patch

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTPREFIX_qtwebkit}
               -DCMAKE_BUILD_TYPE=MinSizeRel
               ${GLOBAL_PROFILE}
               -DCMAKE_SYSTEM_PREFIX_PATH=${EXTPREFIX_qtwebkit}
               -DENABLE_GEOLOCATION=OFF
               -DENABLE_DEVICE_ORIENTATION=OFF
               -DENABLE_QT_WEBCHANNEL=OFF
               -DENABLE_MEDIA_SOURCE=OFF
               -DENABLE_VIDEO=OFF
               -DENABLE_WEB_AUDIO=OFF
               -DENABLE_PRINT_SUPPORT=OFF
               -DENABLE_QT_GESTURE_EVENTS=OFF
               -DENABLE_INSPECTOR_UI=ON
               -DENABLE_API_TESTS=OFF
               -DENABLE_TOOLS=OFF
               -DENABLE_JIT=OFF
               -DUSE_GSTREAMER=OFF
               -DUSE_MEDIA_FOUNDATION=OFF
               -DUSE_QT_MULTIMEDIA=OFF
               -DUSE_LIBHYPHEN=OFF
               -DUSE_SYSTEM_MALLOC=1
               -DUSE_WOFF2=ON
               -DPORT=Qt
               -Wno-dev

    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    ALWAYS 0
)
