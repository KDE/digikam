#!/bin/sh

# Copyright (c) 2008-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
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

export Exiv2Options='-DBUILD_SHARED_LIBS=ON \
                     -DEXIV2_ENABLE_XMP=ON \
                     -DEXIV2_ENABLE_EXTERNAL_XMP=OFF \
                     -DEXIV2_ENABLE_PNG=ON \
                     -DEXIV2_ENABLE_NLS=ON \
                     -DEXIV2_ENABLE_PRINTUCS2=ON \
                     -DEXIV2_ENABLE_LENSDATA=ON \
                     -DEXIV2_ENABLE_VIDEO=OFF \
                     -DEXIV2_ENABLE_WEBREADY=OFF \
                     -DEXIV2_ENABLE_DYNAMIC_RUNTIME=OFF \
                     -DEXIV2_ENABLE_CURL=OFF \
                     -DEXIV2_ENABLE_SSH=OFF \
                     -DEXIV2_BUILD_SAMPLES=OFF \
                     -DEXIV2_BUILD_PO=OFF \
                     -DEXIV2_BUILD_EXIV2_COMMAND=OFF \
                     -DEXIV2_BUILD_UNIT_TESTS=OFF \
                     -DEXIV2_BUILD_DOC=OFF \
                     -DEXIV2_TEAM_EXTRA_WARNINGS=OFF \
                     -DEXIV2_TEAM_WARNINGS_AS_ERRORS=OFF \
                     -DEXIV2_TEAM_USE_SANITIZERS=OFF \
                     -DEXIV2_ENABLE_WIN_UNICODE=OFF'

cmake -G "$MAKEFILES_TYPE" . \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -Wno-dev \
      $Exiv2Options \
      ..

