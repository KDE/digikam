#!/bin/bash

# Copyright (c) 2013-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

########################################################################

# Absolute path where are downloaded all tarballs to compile.
DOWNLOAD_DIR="`pwd`/temp.dwnld"

# Absolute path where are compiled all tarballs
BUILDING_DIR="`pwd`/temp.build"

########################################################################

# Minimum MacOS target for backward binary compatibility
# This require to install older MacOS SDKs with Xcode.
# See this url to download a older SDK archive :
#
# https://github.com/phracker/MacOSX-SDKs/releases
#
# Uncompress the archive to /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/
# and adjust the property "MinimumSDKVersion" from /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Info.plist

# Possible values:
# 10.11 : El Capitan   (tested)
# 10.10 : Yosemite     (tested)
# 10.9  : Mavericks    (tested)
# 10.8  : MountainLion (tested)
# 10.7  : Lion         (untested)
# 10.6  : SnowLeopard  (untested)
# Older values cannot be set as it do no support x86_64.
OSX_MIN_TARGET="10.8"

# Directory where not relocable bundle will be built, and where it will be installed by packaging script
INSTALL_PREFIX="/opt/digikam"

# Macports configuration
MP_URL="https://distfiles.macports.org/MacPorts/"
MP_BUILDTEMP=~/mptemp

# Uncomment this line to force a specific version of Macports to use, else lastest will be used.
#MP_VERSION="2.3.3"

# digiKam tarball information
DK_URL="http://download.kde.org/stable/digikam"

# Location to build source code.
DK_BUILDTEMP=~/dktemp

# digiKam tag version from git. Official tarball do not include extra shared libraries.
# The list of tags can be listed with this url: https://quickgit.kde.org/?p=digikam.git&a=tags
# If you want to package current implemntation from git, use "master" as tag.
#DK_VERSION=v5.5.0
#DK_VERSION=master
DK_VERSION=development/6.0.0

# Installer sub version to differentiates newer updates of the installer itself, even if the underlying application hasn’t changed.
#DK_EPOCH="-01"
# Epoch with time-stamp for pre-release bundle in ISO format
DK_EPOCH="-`date "+%Y%m%dT%H%M%S"`"

# Installer will include or not digiKam debug symbols
DK_DEBUG=1

# Upload automatically bundle to files.kde.org (pre-release only).
DK_UPLOAD=1
DK_UPLOADURL="digikam@racnoss.kde.org"
DK_UPLOADDIR="/srv/archives/files/digikam/"
