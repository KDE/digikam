#! /bin/bash

# Script to build a bundle MXE installation with all digiKam low level dependencies in a dedicated directory.
#
# Copyright (c) 2015-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

#################################################################################################
# Manage script traces to log file

mkdir -p ./logs
exec > >(tee ./logs/build-mxe.full.log) 2>&1

#################################################################################################

echo "01-build-mxe.sh : build a bundle MXE install with digiKam dependencies."
echo "-----------------------------------------------------------------------"

#################################################################################################
# Pre-processing checks

. ./config.sh
. ./common.sh
StartScript
ChecksCPUCores

#################################################################################################

# Pathes rules
ORIG_PATH="$PATH"
ORIG_WD="`pwd`"

export PATH=$MXE_BUILDROOT/usr/bin:$MXE_INSTALL_PREFIX/qt5/bin:$PATH

###############################################################################################
# Check if a previous bundle already exist

CONTINUE_INSTALL=0

if [ -d "$MXE_BUILDROOT" ] ; then

    read -p "$MXE_BUILDROOT already exist. Do you want to remove it or to continue an aborted previous installation ? [(r)emove/(c)ontinue/(s)top] " answer

    if echo "$answer" | grep -iq "^r" ;then

        echo "---------- Removing existing $MXE_BUILDROOT"
#        chmod +w "$MXE_BUILDROOT/usr/readonly"
#        chattr -i "$MXE_BUILDROOT/usr/readonly/.gitkeep"
        rm -rf "$MXE_BUILDROOT"

    elif echo "$answer" | grep -iq "^c" ;then

        echo "---------- Continue aborted previous installation in $MXE_BUILDROOT"
        CONTINUE_INSTALL=1

    else

        echo "---------- Aborting..."
        exit;

    fi

fi

if [[ $CONTINUE_INSTALL == 0 ]]; then

    #################################################################################################
    # Checkout latest MXE from github

    git clone $MXE_GIT_URL $MXE_BUILDROOT

fi

#################################################################################################
# MXE git revision to use

cd $MXE_BUILDROOT

if [[ $MXE_GIT_REVISION == "master" ]]; then
    echo -e "\n"
    echo "---------- Updating MXE"
    git pull
else
    echo -e "\n"
    echo "---------- Checkout MXE revision to $MXE_GIT_REVISION"
    git checkout $MXE_GIT_REVISION
fi

#################################################################################################
# Dependencies build and installation

echo -e "\n"
echo "---------- Building digiKam low level dependencies with MXE"

make MXE_TARGETS=$MXE_BUILD_TARGETS \
     gcc \
     gdb \
     cmake \
     gettext \
     freeglut \
     libxml2 \
     libxslt \
     libjpeg-turbo \
     libpng \
     tiff \
     boost \
     expat \
     lcms \
     liblqr-1 \
     eigen \
     jasper \
     zlib \
     expat \
     mman-win32 \
     pthreads \
     qtbase \
     qttranslations \
     qtimageformats \
     qtwebkit \
     qttools \
     qtwinextras \
     qtscript \
     ffmpeg \
     openal \
     lensfun \
     libical

echo -e "\n"

#################################################################################################

echo -e "\n"
echo "---------- Building digiKam 3rd-party dependencies with MXE"

# Create the build dir for the 3rdparty deps
if [ ! -d $BUILDING_DIR ] ; then
    mkdir -p $BUILDING_DIR
fi
if [ ! -d $DOWNLOAD_DIR ] ; then
    mkdir -p $DOWNLOAD_DIR
fi

cd $BUILDING_DIR
rm -rf $BUILDING_DIR/* || true

${MXE_BUILD_TARGETS}-cmake $ORIG_WD/../3rdparty \
                           -DMXE_TOOLCHAIN=${MXE_TOOLCHAIN} \
                           -DMXE_BUILDROOT=${MXE_BUILDROOT} \
                           -DMXE_ARCHBITS=${MXE_ARCHBITS} \
                           -DMXE_INSTALL_PREFIX=${MXE_INSTALL_PREFIX} \
                           -DCMAKE_BUILD_TYPE=RelWithDebInfo \
                           -DCMAKE_COLOR_MAKEFILE=ON \
                           -DCMAKE_INSTALL_PREFIX=${MXE_INSTALL_PREFIX} \
                           -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON \
                           -DCMAKE_TOOLCHAIN_FILE=${MXE_TOOLCHAIN} \
                           -DCMAKE_FIND_PREFIX_PATH=${CMAKE_PREFIX_PATH} \
                           -DCMAKE_SYSTEM_INCLUDE_PATH=${CMAKE_PREFIX_PATH}/include \
                           -DCMAKE_INCLUDE_PATH=${CMAKE_PREFIX_PATH}/include \
                           -DCMAKE_LIBRARY_PATH=${CMAKE_PREFIX_PATH}/lib \
                           -DZLIB_ROOT=${CMAKE_PREFIX_PATH} \
                           -DINSTALL_ROOT=${MXE_INSTALL_PREFIX} \
                           -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOAD_DIR

# Low level libraries
# NOTE: The order to compile each component here is very important.

${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_opencv     -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_exiv2      -- -j$CPU_CORES
${MXE_BUILD_TARGETS}-cmake --build . --config RelWithDebInfo --target ext_qtav       -- -j$CPU_CORES

#################################################################################################

export PATH=$ORIG_PATH

TerminateScript
