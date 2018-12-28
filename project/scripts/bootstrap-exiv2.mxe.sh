#!/bin/bash

# Copyright (c) 2008-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Arguments : $1 : MXE build root path to MXE bundle dir (default ./project/mxe/build.win32).
#             $2 : build type : 'debugfull' to hack (default), 'release' for production, relwithdebinfo for packaging.

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

ORIG_WD="`pwd`"

MXE_BUILDROOT=$1

if [[ $MXE_BUILDROOT == "" ]]; then

        echo "MXE target build directory is missing!"
        exit -1

fi

if [[ $MXE_BUILDROOT == *.win32 ]]; then

    echo "MXE target : 32 bits shared"
    MXE_BUILD_TARGETS="i686-w64-mingw32.shared"

elif [[ $MXE_BUILDROOT == *.win64 ]]; then

    echo "MXE target : 64 bits shared"
    MXE_BUILD_TARGETS="x86_64-w64-mingw32.shared"

else

    echo "Invalid MXE target!"
    exit -1

fi

BUILD_TYPE=$2

if [ "$BUILD_TYPE" = "" ]; then
    BUILD_TYPE=RelWithDebInfo
fi

MXE_INSTALL_PREFIX=${MXE_BUILDROOT}/usr/${MXE_BUILD_TARGETS}/
MXE_TOOLCHAIN=${MXE_INSTALL_PREFIX}/share/cmake/mxe-conf.cmake

# Exiv2 configuration options for cmake.
OPTIONS='-DBUILD_SHARED_LIBS=ON \
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
         -DEXIV2_BUILD_EXIV2_COMMAND=ON \
         -DEXIV2_BUILD_UNIT_TESTS=OFF \
         -DEXIV2_BUILD_DOC=OFF \
         -DEXIV2_TEAM_EXTRA_WARNINGS=OFF \
         -DEXIV2_TEAM_WARNINGS_AS_ERRORS=OFF \
         -DEXIV2_TEAM_USE_SANITIZERS=OFF \
         -DEXIV2_ENABLE_WIN_UNICODE=OFF'

echo "Installing to $MXE_BUILDROOT for target $MXE_BUILD_TARGETS with build mode $BUILD_TYPE and configure options $OPTIONS"

# Pathes rules
ORIG_PATH="$PATH"
export PATH=$MXE_BUILDROOT/usr/bin:$PATH

if [ ! -d "build" ]; then
    mkdir $ORIG_WD/build
fi

cd $ORIG_WD/build

${MXE_BUILD_TARGETS}-cmake -G "Unix Makefiles" . \
                           -DMXE_TOOLCHAIN=${MXE_TOOLCHAIN} \
                           -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
                           -DCMAKE_COLOR_MAKEFILE=ON \
                           -DCMAKE_INSTALL_PREFIX=${MXE_INSTALL_PREFIX} \
                           -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
                           -DCMAKE_TOOLCHAIN_FILE=${MXE_TOOLCHAIN} \
                           -DCMAKE_FIND_PREFIX_PATH=${CMAKE_PREFIX_PATH} \
                           -DCMAKE_SYSTEM_INCLUDE_PATH=${CMAKE_PREFIX_PATH}/include \
                           -DCMAKE_INCLUDE_PATH=${CMAKE_PREFIX_PATH}/include \
                           -DCMAKE_LIBRARY_PATH=${CMAKE_PREFIX_PATH}/lib \
                           -DZLIB_ROOT=${CMAKE_PREFIX_PATH} \
                           ${OPTIONS} \
                           -Wno-dev \
                           ..

CMAKE_VAL_RET=$?

export PATH=$ORIG_PATH

exit $CMAKE_VAL_RET
