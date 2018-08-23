#!/bin/sh

# Copyright (c) 2008-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Copy this script on root folder where are source code

export PATH=$QTDIR/bin:$PATH
export PKG_CONFIG_PATH=/usr/lib/pkgconfig

#export VERBOSE=1

# We will work on command line using MinGW compiler
export MAKEFILES_TYPE='Unix Makefiles'

if [ ! -d "build" ]; then
    mkdir build
fi

cd build

export Exiv2Options='BUILD_SHARED_LIBS=ON \
                     EXIV2_ENABLE_XMP=ON \
                     EXIV2_ENABLE_EXTERNAL_XMP=OFF \
                     EXIV2_ENABLE_PNG=ON \
                     EXIV2_ENABLE_NLS=ON \
                     EXIV2_ENABLE_PRINTUCS2=ON \
                     EXIV2_ENABLE_LENSDATA=ON \
                     EXIV2_ENABLE_COMMERCIAL=OFF \
                     EXIV2_ENABLE_VIDEO=OFF \
                     EXIV2_ENABLE_WEBREADY=OFF \
                     EXIV2_ENABLE_DYNAMIC_RUNTIME=OFF \
                     EXIV2_ENABLE_WIN_UNICODE=OFF \
                     EXIV2_ENABLE_CURL=OFF \
                     EXIV2_ENABLE_SSH=OFF \
                     EXIV2_BUILD_SAMPLES=OFF \
                     EXIV2_BUILD_PO=ON \
                     EXIV2_BUILD_EXIV2_COMMAND=OFF \
                     EXIV2_BUILD_UNIT_TESTS=OFF \
                     EXIV2_BUILD_DOC=OFF \
                     EXIV2_TEAM_EXTRA_WARNINGS=OFF \
                     EXIV2_TEAM_WARNINGS_AS_ERRORS=OFF'

cmake -G "$MAKEFILES_TYPE" . \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -Wno-dev \
      $Exiv2Options \
      ..

