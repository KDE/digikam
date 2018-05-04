#!/bin/sh

# Copyright (c) 2013-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

PDIR="`dirname $(dirname $PWD)`"
TITLE="`basename $PDIR`"

cd ../..

export CMAKE_BINARY="scan-build cmake"
./bootstrap.linux

cd build
scan-build -o ./ --html-title $TITLE -v -k make -j4

