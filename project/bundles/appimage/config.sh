#!/bin/bash

# Copyright (c) 2015-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

########################################################################

# Absolute path where are downloaded all tarballs to compile.
DOWNLOAD_DIR="/d"

# Absolute path where are compiled all tarballs
BUILDING_DIR="/b"

########################################################################


# Location to build source code.
DK_BUILDTEMP=$BUILDING_DIR/dktemp

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
