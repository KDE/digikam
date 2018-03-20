#! /bin/bash

# Script to give a console with port CLI tool branched to MAcports bundle repository
# This script must be run as sudo
#
# Copyright (c) 2015-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

# Ask to run as root
(( EUID != 0 )) && exec sudo -- "$0" "$@"

# halt on error
set -e

. ../config.sh
. ../common.sh

#################################################################################################

# Pathes rules
ORIG_PATH="$PATH"
export PATH=$INSTALL_PREFIX/bin:/$INSTALL_PREFIX/sbin:$ORIG_PATH

CommonChecks

#################################################################################################

port

#################################################################################################

export PATH=$ORIG_PATH
