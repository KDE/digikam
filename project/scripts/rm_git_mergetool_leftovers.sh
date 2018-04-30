#!/bin/sh

# Copyright (c) 2008-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Use this script to clean up your working dir before building the API documentation

E_ERROR=1

function usage() {
    echo "Usage: $(basename $0) <path>"
}

if [ $# -ne 1 ]; then
    usage
    exit $E_ERROR
fi

if [ -d $1 ]; then
    find $1 -type f -name "*BASE*" \
        -or -name "*REMOTE*" \
        -or -name "*LOCAL*" \
        -or -name "*.orig" \
        -exec rm {} \;
    exit $?
else
    echo "'$1' is not a valid directory. Make sure you have permissions to access this location."
    exit $E_ERROR
fi

