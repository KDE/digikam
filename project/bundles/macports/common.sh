#!/bin/bash

# Copyright (c) 2013-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

########################################################################
# Check if run as root
ChecksRunAsRoot()
{

if [[ $EUID -ne 0 ]]; then
    echo "This script should be run as root using sudo command."
    exit 1
else
    echo "Check run as root passed..."
fi

}

########################################################################
# Check if Xcode Command Line tools are installed
ChecksXCodeCLI()
{

xcode-select --print-path

if [[ $? -ne 0 ]]; then
    echo "XCode CLI tools are not installed"
    echo "See http://www.macports.org/install.php for details."
    exit 1
else
    echo "Check XCode CLI tools passed..."
fi

export MACOSX_DEPLOYMENT_TARGET=$OSX_MIN_TARGET

OSX_MAJOR=`echo $MACOSX_DEPLOYMENT_TARGET | awk -F '.' '{print $1 "." $2}'| cut -d . -f 2`

if [[ $OSX_MAJOR -lt 9 ]]; then
    export CXXFLAGS=-stdlib=libc++
fi

echo "Target OSX minimal version: $MACOSX_DEPLOYMENT_TARGET"
}

########################################################################
# Check if Macports is installed
ChecksMacports()
{

which port

if [[ $? -ne 0 ]]; then
    echo "Macports is not installed"
    echo "See http://www.macports.org/install.php for details."
    exit 1
else
    echo "Check Macports passed..."
fi

}

########################################################################
# Check CPU core available (Linux or MacOS)
ChecksCPUCores()
{

CPU_CORES=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu)

if [[ $CPU_CORES -gt 1 ]]; then
    CPU_CORES=$((CPU_CORES-1))
fi

echo "CPU Cores to use : $CPU_CORES"

}

########################################################################
# Performs All Checks
CommonChecks()
{

ChecksRunAsRoot
ChecksXCodeCLI
ChecksMacports

}

########################################################################
# For time execution measurement ; startup
StartScript()
{

BEGIN_SCRIPT=$(date +"%s")

}

########################################################################
# For time execution measurement : shutdown
TerminateScript()
{

TERMIN_SCRIPT=$(date +"%s")
difftimelps=$(($TERMIN_SCRIPT-$BEGIN_SCRIPT))
echo "Elaspsed time for script execution : $(($difftimelps / 3600 )) hours $((($difftimelps % 3600) / 60)) minutes $(($difftimelps % 60)) seconds"

}

########################################################################
# Set strings with detected MacOS info :
#    $MAJOR_OSX_VERSION : detected MacOS major ID (as 7 for 10.7 or 10 for 10.10)
#    $OSX_CODE_NAME     : detected MacOS code name
OsxCodeName()
{

MAJOR_OSX_VERSION=$(sw_vers -productVersion | awk -F '.' '{print $1 "." $2}'| cut -d . -f 2)

if [[ $MAJOR_OSX_VERSION == "12" ]]
    then OSX_CODE_NAME="Sierra"
elif [[ $MAJOR_OSX_VERSION == "11" ]]
    then OSX_CODE_NAME="ElCapitan"
elif [[ $MAJOR_OSX_VERSION == "10" ]]
    then OSX_CODE_NAME="Yosemite"
elif [[ $MAJOR_OSX_VERSION == "9" ]]
    then OSX_CODE_NAME="Mavericks"
elif [[ $MAJOR_OSX_VERSION == "8" ]]
    then OSX_CODE_NAME="MountainLion"
elif [[ $MAJOR_OSX_VERSION == "7" ]]
    then OSX_CODE_NAME="Lion"
elif [[ $MAJOR_OSX_VERSION == "6" ]]
    then OSX_CODE_NAME="SnowLeopard"
elif [[ $MAJOR_OSX_VERSION == "5" ]]
    then OSX_CODE_NAME="Leopard"
elif [[ $MAJOR_OSX_VERSION == "4" ]]
    then OSX_CODE_NAME="Tiger"
elif [[ $MAJOR_OSX_VERSION == "3" ]]
    then OSX_CODE_NAME="Panther"
elif [[ $MAJOR_OSX_VERSION == "2" ]]
    then OSX_CODE_NAME="Jaguar"
elif [[ $MAJOR_OSX_VERSION == "1" ]]
    then OSX_CODE_NAME="Puma"
elif [[ $MAJOR_OSX_VERSION == "0" ]]
    then OSX_CODE_NAME="Cheetah"
fi

echo -e "---------- Detected OSX version 10.$MAJOR_OSX_VERSION and code name $OSX_CODE_NAME"

}

#################################################################################################
# Relocate list of binaries files.
# Replace INSTALL_PREFIX by @rpath in library pathes dependencies registered in bin file.
# List of bin files to patch is passed as first argument.
RelocateBinaries()
{

RPATHSTR="@rpath"

FILESLIST=("${!1}")

#echo "Relocate list: ${FILESLIST[@]}"

for FILE in ${FILESLIST[@]} ; do

    echo "Relocate binary $FILE"

    # List all external dependencies starting with INSTALL_PREFIX
    DEPS=$(otool -L $FILE | grep $INSTALL_PREFIX | awk -F ' \\\(' '{print $1}')

    # For each file from bin list, we replace the absolute path to external dependency with a relative path
    # NOTE: releative path must be resolved in main executable later.
    for EXTLIB in $DEPS ; do

        RPATHLIB=${EXTLIB/$INSTALL_PREFIX/$RPATHSTR}
#        echo "   $EXTLIB ==> $RPATHLIB"
        install_name_tool -change $EXTLIB $RPATHLIB $FILE

    done

done

}
