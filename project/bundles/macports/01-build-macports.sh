#! /bin/bash

# Script to build a bundle Macports installation with all digiKam dependencies in a dedicated directory
# This script must be run as sudo
#
# Copyright (c) 2015,      Shanti, <listaccount at revenant dot org>
# Copyright (c) 2015-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

# Ask to run as root
(( EUID != 0 )) && exec sudo -- "$0" "$@"

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

#################################################################################################
# Manage script traces to log file

mkdir -p ./logs
exec > >(tee ./logs/build-macports.full.log) 2>&1

#################################################################################################

echo "01-build-macports.sh : build a bundle Macports install with digiKam dependencies."
echo "---------------------------------------------------------------------------------"

#################################################################################################
# Pre-processing checks

. ./config.sh
. ./common.sh
StartScript
ChecksRunAsRoot
ChecksXCodeCLI
ChecksCPUCores
OsxCodeName

#################################################################################################

# Pathes rules
ORIG_PATH="$PATH"
ORIG_WD="`pwd`"

export PATH=$INSTALL_PREFIX/bin:/$INSTALL_PREFIX/sbin:/$INSTALL_PREFIX/libexec/qt5/bin:$ORIG_PATH

#################################################################################################
# Check if /opt exists and standard Macports install path

if [ -d "/opt" ] ; then
    if [ -d "/opt/local" ] ; then
        echo "---------- A standard Macports install exists on /opt/local."
        echo "           To prevent wrong links from this bundle to this repository"
        echo "           this one must be disabled (moving to /opt/local.back for ex)."
        echo "---------- Aborting..."
        exit;
    fi
else
    echo "---------- /opt do not exist, creating"

    mkdir "/opt"

    if [ $? -ne 0 ] ; then
        echo "---------- Cannot create /opt directory."
        echo "---------- Aborting..."
        exit;
    fi
fi

#################################################################################################
# Check if a previous bundle already exist

CONTINUE_INSTALL=0

if [ -d "$INSTALL_PREFIX" ] ; then

    read -p "$INSTALL_PREFIX already exist. Do you want to remove it or to continue an aborted previous installation ? [(r)emove/(c)ontinue/(s)top] " answer

    if echo "$answer" | grep -iq "^r" ;then

        echo "---------- Removing existing $INSTALL_PREFIX"
        rm -rf "$INSTALL_PREFIX"

    elif echo "$answer" | grep -iq "^c" ;then

        echo "---------- Continue aborted previous installation in $INSTALL_PREFIX"
        CONTINUE_INSTALL=1

    else

        echo "---------- Aborting..."
        exit;

    fi

fi

if [[ $CONTINUE_INSTALL == 0 ]]; then

    #################################################################################################
    # Target directory creation

    echo "---------- Creating $INSTALL_PREFIX"

    mkdir "$INSTALL_PREFIX"

    if [ $? -ne 0 ] ; then

         echo "---------- Cannot create target install directory $INSTALL_PREFIX"
         echo "---------- Aborting..."
         exit;

    fi

    #################################################################################################
    # Check latest Macports version available if necessary

    if [ -z $MP_VERSION ] ; then

        MP_LASTEST_VER=$(curl $MP_URL | \
            egrep -o 'href="MacPorts-[0-9]+\.[0-9]+\.[0-9]+' | \
            sed 's/^href="MacPorts-//' | \
            sort -t. -rn -k1,1 -k2,2 -k3,3 | head -1)

        if [ -z $MP_LASTEST_VER ] ; then
            echo "---------- Cannot check the lastest Macports verion from $MP_URL"
            echo "---------- Aborting..."
            exit;
        fi

        echo "---------- Detected lastest Macports version : $MP_LASTEST_VER"

        MP_VERSION=$MP_LASTEST_VER

    fi

    #################################################################################################
    # Build Macports in temporary directory and installation

    if [ -d "$MP_BUILDTEMP" ] ; then
    echo "---------- Removing existing $MP_BUILDTEMP"
    rm -rf "$MP_BUILDTEMP"
    fi

    echo "---------- Creating $MP_BUILDTEMP"
    mkdir "$MP_BUILDTEMP"

    if [ $? -ne 0 ] ; then
        echo "---------- Cannot create temporary directory $MP_BUILDTEMP to compile Macports"
        echo "---------- Aborting..."
        exit;
    fi

    cd "$MP_BUILDTEMP"
    echo -e "\n\n"

    echo "---------- Downloading MacPorts $MP_VERSION"
    curl -o "MacPorts-$MP_VERSION.tar.bz2" "$MP_URL/MacPorts-$MP_VERSION.tar.bz2"
    tar jxvf MacPorts-$MP_VERSION.tar.bz2
    cd MacPorts-$MP_VERSION
    echo -e "\n\n"

    echo "---------- Configuring MacPorts"

    ./configure --prefix="$INSTALL_PREFIX" \
                --with-applications-dir="$INSTALL_PREFIX/Applications" \
                --with-no-root-privileges \
                --with-install-user="$(id -n -u)" \
                --with-install-group="$(id -n -g)" 

    echo -e "\n\n"

    echo "---------- Building MacPorts"
    make -j$CPU_CORES
    echo -e "\n\n"

    echo "---------- Installing MacPorts"
    echo -e "\n\n"
    make install && cd "$ORIG_WD" && rm -rf "$MP_BUILDTEMP"

    cat << EOF >> "$INSTALL_PREFIX/etc/macports/macports.conf"
+no_root -startupitem
startupitem_type none
startupitem_install no
macosx_deployment_target $OSX_MIN_TARGET
EOF

fi

#################################################################################################
# Macports update

echo -e "\n"
echo "---------- Updating MacPorts"
port -v selfupdate

if [[ $CONTINUE_INSTALL == 0 ]]; then

#    port -v upgrade outdated
    echo -e "\n"

    #echo "---------- Modifying net-snmp portfile to install when not root"
    #sed -e "/install.asroot/ s|yes|no|" -i ".orig" "`port file net-snmp`"

fi

#################################################################################################
# Dependencies build and installation

echo -e "\n"
echo "---------- Building digiKam dependencies with Macports"

# With OSX less than El Capitan, we need a more recent Clang compiler than one provided by XCode.

if [[ $MAJOR_OSX_VERSION -lt 10 ]]; then

    echo "---------- Install more recent Clang compiler from Macports for specific ports"
    port install clang_select
    port install clang-3.4
    port select --set clang mp-clang-3.4
fi

echo -e "\n"

port install cmake \
             libpng \
             jpeg \
             tiff \
             boost \
             eigen3 +gcc5 \
             gettext \
             libusb \
             libgphoto2 \
             jasper \
             lcms2 \
             expat \
             libxml2 \
             libxslt \
             libical \
             lensfun \
             bison \
             ffmpeg \
             sane-backends \
             qt58 \
             qt58-qtimageformats \
             qt58-qtwebkit \
             qt58-sqlite-plugin \
             qt58-mysql-plugin \
             qt58-qtscript \
             wget +ssl

echo -e "\n"

echo -e "---------- Compilation logs of Macports packages with suspicious installation\n"
find $INSTALL_PREFIX/var/macports/logs/ -name main.log
echo -e "\n----------"

echo -e "\n"

#################################################################################################

# Create the build dir for the 3rdparty deps
if [ ! -d $BUILDING_DIR ] ; then
    mkdir $BUILDING_DIR
fi
if [ ! -d $DOWNLOAD_DIR ] ; then
    mkdir $DOWNLOAD_DIR
fi

cd $BUILDING_DIR

rm -rf $BUILDING_DIR/* || true

cmake $ORIG_WD/../3rdparty \
       -DCMAKE_INSTALL_PREFIX:PATH=$INSTALL_PREFIX \
       -DINSTALL_ROOT=$INSTALL_PREFIX \
       -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOAD_DIR \
       -Wno-dev

cmake --build . --config RelWithDebInfo --target ext_opencv -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_exiv2  -- -j$CPU_CORES
cmake --build . --config RelWithDebInfo --target ext_qtav   -- -j$CPU_CORES

#################################################################################################

export PATH=$ORIG_PATH

TerminateScript
