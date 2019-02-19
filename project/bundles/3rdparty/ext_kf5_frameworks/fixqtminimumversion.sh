#!/bin/bash

# Copyright (c) 2013-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

########################################################################
# Fix Qt Minimum version to build bundle.

LC_CTYPE=C find . -name "*.txt" -type f -exec sed -i '' -e 's/set(REQUIRED_QT_VERSION 5.10.0)/set(REQUIRED_QT_VERSION 5.9.7)/g' {} \;

