#!/bin/bash

# Copyright (c) 2013-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

krazy2all --check includes \
          --export textedit \
          --title digiKam \
          --no-brief \
          --strict all \
          --priority all \
          --verbose \
          --topdir ../../core \
          --exclude-types perl,python \
          --outfile report.krazy