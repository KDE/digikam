#!/bin/bash

# Script to build a CentOS 6 installation to compile an AppImage bundle of digiKam.
# This script must be run as sudo
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
exec > >(tee ./logs/build-centos6.full.log) 2>&1

#################################################################################################

echo "01-build-centos6.sh : build a CentOS 6 installation to compile an AppImage of digiKam."
echo "--------------------------------------------------------------------------------------"

#################################################################################################
# Pre-processing checks

. ./config.sh
. ./common.sh
ChecksRunAsRoot
StartScript
ChecksCPUCores
CentOS6Adjustments
ORIG_WD="`pwd`"

#################################################################################################

echo -e "---------- Update Linux CentOS 6\n"

if [[ "$(arch)" = "x86_64" ]] ; then
    yum upgrade ca-certificates --disablerepo=epel
fi

if [[ ! -f /etc/yum.repos.d/epel.repo ]] ; then

    yum install epel-release

    # we need to be up to date in order to install the xcb-keysyms dependency
    yum -y update
fi

#################################################################################################

echo -e "---------- Install New Development Packages\n"

# Packages for base dependencies and Qt5.
yum -y install wget \
               tar \
               bzip2 \
               gettext \
               git \
               subversion \
               libtool \
               which \
               fuse \
               automake \
               mesa-libEGL \
               cmake3 \
               gcc-c++ \
               patch \
               libxcb \
               xcb-util \
               xkeyboard-config \
               gperf \
               ruby \
               bison \
               flex \
               zlib-devel \
               expat-devel \
               fuse-devel \
               libtool-ltdl-devel \
               glib2-devel \
               glibc-headers \
               mysql-devel \
               eigen3-devel \
               openssl-devel \
               cppunit-devel \
               libstdc++-devel \
               freetype-devel \
               fontconfig-devel \
               libxml2-devel \
               libstdc++-devel \
               libXrender-devel \
               lcms2-devel \
               xcb-util-keysyms-devel \
               libXi-devel \
               mesa-libGL-devel \
               mesa-libGLU-devel \
               libxcb-devel \
               xcb-util-devel \
               glibc-devel \
               libudev-devel \
               libicu-devel \
               sqlite-devel \
               libusb-devel \
               libexif-devel \
               libical-devel \
               libxslt-devel \
               xz-devel \
               lz4-devel \
               inotify-tools-devel \
               openssl-devel \
               cups-devel \
               openal-soft-devel \
               libical-devel

#################################################################################################

if [[ ! -f /opt/rh/devtoolset-3/enable ]] ; then

    echo -e "---------- Install New Compiler Tools Set\n"

    if [[ "$(arch)" = "x86_64" ]] ; then

        # Newer compiler than what comes with offcial CentOS 6 (only 64 bits)
        yum -y install centos-release-scl-rh
        yum -y install devtoolset-3-gcc devtoolset-3-gcc-c++

    else

        # Newer compiler that come from Sienctifc Linux for CentOS 6 32 bits
        cd /etc/yum.repos.d
        wget http://linuxsoft.cern.ch/cern/scl/slc6-scl.repo
        yum -y --nogpgcheck install devtoolset-3-gcc devtoolset-3-gcc-c++
        rm -f /etc/yum.repos.d/slc6-scl.repo

    fi

fi


#################################################################################################

# Install new repo to get ffmpeg if necessary

if [[ ! -f /etc/yum.repos.d/nux-dextop.repo ]] ; then

    echo -e "---------- Install Repository for ffmpeg packages\n"

    if [[ "$(arch)" = "x86_64" ]] ; then

        rpm --import http://li.nux.ro/download/nux/RPM-GPG-KEY-nux.ro
        rpm -Uvh http://li.nux.ro/download/nux/dextop/el6/x86_64/nux-dextop-release-0-2.el6.nux.noarch.rpm

    else

        rpm --import http://li.nux.ro/download/nux/RPM-GPG-KEY-nux.ro
        rpm -Uvh http://li.nux.ro/download/nux/dextop/el6/i386/nux-dextop-release-0-2.el6.nux.noarch.rpm

    fi

fi

yum -y install ffmpeg ffmpeg-devel

#################################################################################################

echo -e "---------- Clean-up Old Packages\n"

# Remove system based devel package to prevent conflict with new one.
yum -y erase qt-devel boost-devel libgphoto2 sane-backends libjpeg-devel jasper-devel libpng-devel libtiff-devel

#################################################################################################

echo -e "---------- Prepare CentOS to Compile Extra Dependencies\n"

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

# enable new compiler
. /opt/rh/devtoolset-3/enable

#################################################################################################

cd $BUILDING_DIR

rm -rf $BUILDING_DIR/* || true

cmake3 $ORIG_WD/../3rdparty \
       -DCMAKE_INSTALL_PREFIX:PATH=/usr \
       -DINSTALL_ROOT=/usr \
       -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOAD_DIR

# Low level libraries and Qt5 dependencies
# NOTE: The order to compile each component here is very important.

cmake3 --build . --config RelWithDebInfo --target ext_jpeg       -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_jasper     -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_png        -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_tiff       -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_libgphoto2 -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_sane       -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_boost      -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_opencv     -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_lensfun    -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_qt         -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_qtwebkit   -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_exiv2      -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_qtav       -- -j$CPU_CORES

#################################################################################################

TerminateScript

