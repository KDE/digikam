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

yum -y install epel-release

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
               sqlite-devel \
               libusb-devel \
               libexif-devel \
               libxslt-devel \
               xz-devel \
               lz4-devel \
               inotify-tools-devel \
               openssl-devel \
               cups-devel \
               openal-soft-devel \
               libical-devel

#################################################################################################

if [[ "$(arch)" = "x86_64" ]] ; then

    yum -y --nogpgcheck install centos-release-scl

    if [[ ! -f /opt/rh/python27/enable ]] ; then

        echo -e "---------- Install New Python Interpreter\n"
        yum -y --nogpgcheck install python27

    fi

    if [[ ! -f /opt/rh/rh-ruby24/enable ]] ; then

        echo -e "---------- Install New Ruby Interpreter\n"
        yum -y --nogpgcheck install rh-ruby24

    fi

    if [[ ! -f /opt/rh/devtoolset-6/enable ]] ; then

        echo -e "---------- Install New Compiler Tools Set\n"
        yum -y --nogpgcheck install devtoolset-6

    fi

else

    if [[ ! -f /etc/yum.repos.d/ewdurbin-pythons-el6-epel-6.repo ]] ; then

        echo -e "---------- Install New Python Interpreter\n"

        cd /etc/yum.repos.d
        wget https://copr.fedorainfracloud.org/coprs/ewdurbin/pythons-el6/repo/epel-6/ewdurbin-pythons-el6-epel-6.repo
        yum -y --nogpgcheck install python27
        rm -f /etc/yum.repos.d/ewdurbin-pythons-el6-epel-6.repo

    fi

    if [[ ! -f /etc/profile.d/rvm.sh ]] ; then

        echo -e "---------- Install New Ruby Interpreter\n"

        # Based this tutorial: https://tecadmin.net/install-ruby-latest-stable-centos/

        curl -sSL https://rvm.io/mpapis.asc | gpg2 --import -
        curl -sSL https://rvm.io/pkuczynski.asc | gpg2 --import -
        curl -L get.rvm.io | bash
        source /etc/profile.d/rvm.sh
        rvm reload
        rvm requirements run
        rvm install 2.4
        rvm use 2.4 --default

    fi

    if [[ ! -f /opt/rh/devtoolset-6/enable ]] ; then

        echo -e "---------- Install New Compiler Tools Set\n"
        cd /etc/yum.repos.d
        wget https://copr.fedorainfracloud.org/coprs/mlampe/devtoolset-6/repo/epel-6/mlampe-devtoolset-6-epel-6.repo
        yum -y --nogpgcheck install devtoolset-6-gcc devtoolset-6-gcc-c++
        rm -f /etc/yum.repos.d/mlampe-devtoolset-6-epel-6.repo

    fi

fi

#################################################################################################

# Install new repo to get ffmpeg dependencies

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

yum -y install fdk-aac-devel \
               faac-devel \
               libfribidi-devel \
               lame-devel \
               opencore-amr-devel \
               opus-devel \
               librtmp-devel \
               speex-devel \
               libtheora-devel \
               libvorbis-devel \
               libvpx-devel \
               x264-devel \
               x265-devel \
               xvidcore-devel \
               yasm

#################################################################################################

echo -e "---------- Clean-up Old Packages\n"

# Remove system based devel package to prevent conflict with new one.
yum -y erase qt-devel \
             boost-devel \
             libgphoto2 \
             sane-backends \
             libjpeg-devel \
             jasper-devel \
             libpng-devel \
             libtiff-devel \
             ffmpeg \
             ffmpeg-devel \
             ant \
             pulseaudio-libs-devel \
             fontconfig-devel \
             freetype-devel \
             libicu-devel

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
if [[ -f /opt/rh/devtoolset-6/enable ]] ; then
    . /opt/rh/devtoolset-6/enable
fi

# enable new Python
if [[ -f /opt/rh/python27/enable ]] ; then
    . /opt/rh/python27/enable
fi

# enable new Ruby
if [[ -f /opt/rh/rh-ruby24/enable ]] ; then
    . /opt/rh/rh-ruby24/enable
fi

#################################################################################################

cd $BUILDING_DIR

rm -rf $BUILDING_DIR/* || true

cmake3 $ORIG_WD/../3rdparty \
       -DCMAKE_INSTALL_PREFIX:PATH=/usr \
       -DINSTALL_ROOT=/usr \
       -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOAD_DIR

# Low level libraries and Qt5 dependencies
# NOTE: The order to compile each component here is very important.

cmake3 --build . --config RelWithDebInfo --target ext_jpeg          -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_jasper        -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_png           -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_tiff          -- -j$CPU_CORES
#cmake3 --build . --config RelWithDebInfo --target ext_openssl       -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_freetype      -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_fontconfig    -- -j$CPU_CORES    # depend of freetype
cmake3 --build . --config RelWithDebInfo --target ext_libicu        -- -j$CPU_CORES

cmake3 --build . --config RelWithDebInfo --target ext_qt            -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_qtwebkit      -- -j$CPU_CORES

cmake3 --build . --config RelWithDebInfo --target ext_boost         -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_libass        -- -j$CPU_CORES    # depend of fontconfig
cmake3 --build . --config RelWithDebInfo --target ext_ffmpeg        -- -j$CPU_CORES    # depend of libass
cmake3 --build . --config RelWithDebInfo --target ext_qtav          -- -j$CPU_CORES    # depend of qt and ffmpeg

cmake3 --build . --config RelWithDebInfo --target ext_libgphoto2    -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_sane          -- -j$CPU_CORES    # depend of libgphoto2
cmake3 --build . --config RelWithDebInfo --target ext_exiv2         -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_opencv        -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_lensfun       -- -j$CPU_CORES
#cmake3 --build . --config RelWithDebInfo --target ext_liblqr        -- -j$CPU_CORES
cmake3 --build . --config RelWithDebInfo --target ext_linuxdeployqt -- -j$CPU_CORES    # depend of qt

#################################################################################################

TerminateScript
