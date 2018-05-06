#!/bin/bash

# Copyright (c) 2013-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Run Krazy static analyzer on whole digiKam source code.
# https://github.com/Krazy-collection/krazy
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

krazy2all --export xml \
          --title digiKam \
          --no-brief \
          --strict all \
          --priority all \
          --verbose \
          --topdir ../../ \
          --check defines \
          --outfile report.krazy