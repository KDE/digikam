#!/bin/bash

# Copyright (c) 2013-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

########################################################################
# Fix QStandardPaths problem with non bundle like place to sore data under OSX.

LC_CTYPE=C find . -name "*.cpp" -type f -exec sed -i '' -e 's/GenericDataLocation/AppDataLocation/g' {} \;

