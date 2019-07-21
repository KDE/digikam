#!/bin/bash

# Script to build a CentOS 6 installation to compile an AppImage bundle of digiKam.
# This script must be run as sudo
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
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
exec > >(tee ./logs/build-host.full.log) 2>&1

#################################################################################################

echo "01-build-host.sh : build a Linux host installation to compile an AppImage of digiKam."
echo "--------------------------------------------------------------------------------------"

#################################################################################################
# Pre-processing checks

. ./config.sh
. ./common.sh
ChecksRunAsRoot
StartScript
ChecksCPUCores
HostAdjustments
RegisterRemoteServers
ORIG_WD="`pwd`"

#################################################################################################

echo -e "---------- Update Linux Host\n"

urpmi --auto-update

#################################################################################################

if [[ "$(arch)" = "x86_64" ]] ; then
    LIBSUFFIX=lib64
else
    LIBSUFFIX=lib
fi

echo -e "---------- Install New Development Packages\n"

# Packages for base dependencies and Qt5.
urpmi --auto \
      wget \
      tar \
      bzip2 \
      gettext \
      git \
      subversion \
      libtool \
      which \
      fuse \
      automake \
      cmake \
      gcc-c++ \
      patch \
      libdrm-devel \
      libxcb \
      libxcb-devel \
      xcb-util-keysyms-devel \
      xcb-util-devel \
      xkeyboard-config \
      ${LIBSUFFIX}xcb-util1 \
      ${LIBSUFFIX}xi-devel \
      ${LIBSUFFIX}xtst-devel \
      ${LIBSUFFIX}xrandr-devel \
      ${LIBSUFFIX}xcursor-devel \
      ${LIBSUFFIX}xcomposite-devel \
      ${LIBSUFFIX}xrender-devel \
      ${LIBSUFFIX}mesagl1-devel \
      ${LIBSUFFIX}mesaglu1-devel \
      ${LIBSUFFIX}mesaegl1-devel \
      ${LIBSUFFIX}mesaegl1 \
      xscreensaver \
      gperf \
      ruby \
      bison \
      flex \
      zlib-devel \
      expat-devel \
      fuse-devel \
      ${LIBSUFFIX}ltdl-devel \
      ${LIBSUFFIX}glib2.0-devel \
      glibc-devel \
      mysql-devel \
      eigen3-devel \
      openssl-devel \
      cppunit-devel \
      libstdc++-devel \
      libxml2-devel \
      libstdc++-devel \
      lcms2-devel \
      glibc-devel \
      libudev-devel \
      sqlite-devel \
      ${LIBSUFFIX}usb1.0-devel \
      libexif-devel \
      libxslt-devel \
      xz-devel \
      lz4-devel \
      inotify-tools-devel \
      openssl-devel \
      cups-devel \
      openal-soft-devel \
      libical-devel \
      libcap-devel \
      fontconfig-devel \
      freetype-devel \
      patchelf \
      dpkg \
      python \
      ruby \
      ffmpeg-devel \
      boost-devel \
      gphoto2-devel \
      sane-backends \
      ${LIBSUFFIX}jpeg-devel \
      jasper-devel \
      ${LIBSUFFIX}png-devel \
      ${LIBSUFFIX}tiff-devel \
      ${LIBSUFFIX}icu-devel \
      ${LIBSUFFIX}lqr-devel \
      ${LIBSUFFIX}magick-devel

#################################################################################################

echo -e "---------- Prepare Linux host to Compile Extra Dependencies\n"

# Workaround for: On CentOS 6, .pc files in /usr/lib/pkgconfig are not recognized
# However, this is where .pc files get installed when bulding libraries... (FIXME)
# I found this by comparing the output of librevenge's "make install" command
# between Ubuntu and CentOS 6
ln -sf /usr/share/pkgconfig /usr/lib/pkgconfig

# Make sure we build from the /, parts of this script depends on that. We also need to run as root...
cd /

# Create the build dir for the 3rdparty deps
if [ ! -d $BUILDING_DIR ] ; then
    mkdir $BUILDING_DIR
fi
if [ ! -d $DOWNLOAD_DIR ] ; then
    mkdir $DOWNLOAD_DIR
fi

#################################################################################################

cd $BUILDING_DIR

rm -rf $BUILDING_DIR/* || true

cmake $ORIG_WD/../3rdparty \
      -DCMAKE_INSTALL_PREFIX:PATH=/usr \
      -DINSTALL_ROOT=/usr \
      -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOAD_DIR \
      -DENABLE_QTWEBENGINE=$DK_QTWEBENGINE

# Low level libraries and Qt5 dependencies
# NOTE: The order to compile each component here is very important.

cmake --build . --config RelWithDebInfo --target ext_qt            -- -j$CPU_CORES    # depend of tiff, png, jpeg

if [[ $DK_QTWEBENGINE = 0 ]] ; then
    cmake --build . --config RelWithDebInfo --target ext_qtwebkit  -- -j$CPU_CORES    # depend of Qt and libicu
fi

cmake --build . --config RelWithDebInfo --target ext_qtav          -- -j$CPU_CORES    # depend of qt and ffmpeg

cmake --build . --config RelWithDebInfo --target ext_exiv2         -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_opencv        -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_lensfun       -- -j$CPU_CORES

#################################################################################################

TerminateScript
