#!/bin/bash

# Script to run all MXE based sub-scripts to build Windows installer.
# Possible option : "-f" to force operations without to ask confirmation to user.
#
# Copyright (c) 2013-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

. ./config.sh
. ./common.sh
StartScript

echo "This script will build from scratch the digiKam installer for Windows using MXE."
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

if [[ -d "`pwd`/build.win32" ]] || [[ -d "`pwd`/build.win64" ]] ; then

    if [ "$1" != "-f" ] ; then

        read -p "Previous MXE build already exist and it will be removed. Do you want to continue ? [(c)ontinue/(s)top] " answer

        if echo "$answer" | grep -iq "^s" ; then

            echo "---------- Aborting..."
            exit;

        fi

    fi

    echo "---------- Removing existing MXE build"
    rm -fr `pwd`/build.win32
    rm -fr `pwd`/build.win64

fi

echo "++++++++++++++++   Build 32 bits Installer   ++++++++++++++++++++++++++++++++++"
echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

sed -e "s/MXE_ARCHBITS=64/MXE_ARCHBITS=32/g" ./config.sh > ./tmp.sh ; mv -f ./tmp.sh ./config.sh

./01-build-mxe.sh
./02-build-extralibs.sh
./03-build-digikam.sh
./04-build-installer.sh

echo "++++++++++++++++   Build 64 bits Installer   ++++++++++++++++++++++++++++++++++"
echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

sed -e "s/MXE_ARCHBITS=32/MXE_ARCHBITS=64/g" ./config.sh > ./tmp.sh ; mv -f ./tmp.sh ./config.sh

./01-build-mxe.sh
./02-build-extralibs.sh
./03-build-digikam.sh
./04-build-installer.sh

echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

TerminateScript
