#!/bin/sh

# Copyright (c) 2013-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

cppcheck --enable=warning --force . | grep "]:" | awk '!/libraw/ && !/greycstoration/ && !/xmp_sdk/'
