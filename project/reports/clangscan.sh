#!/bin/sh

# Copyright (c) 2013-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

ORIG_WD="`pwd`"

PDIR="`dirname $(dirname $PWD)`"
TITLE="`basename $PDIR`"

cd ../..

mkdir -p build.scan
cd build.scan

scan-build cmake -G "Unix Makefiles" . \
      -DCMAKE_BUILD_TYPE=debug \
      -DBUILD_TESTING=ON \
      -DDIGIKAMSC_CHECKOUT_PO=OFF \
      -DDIGIKAMSC_CHECKOUT_DOC=OFF \
      -DDIGIKAMSC_COMPILE_PO=OFF \
      -DDIGIKAMSC_COMPILE_DOC=OFF \
      -DDIGIKAMSC_COMPILE_DIGIKAM=ON \
      -DDIGIKAMSC_COMPILE_LIBKSANE=ON \
      -DDIGIKAMSC_COMPILE_LIBMEDIAWIKI=ON \
      -DDIGIKAMSC_COMPILE_LIBKVKONTAKTE=ON \
      -DENABLE_KFILEMETADATASUPPORT=ON \
      -DENABLE_AKONADICONTACTSUPPORT=ON \
      -DENABLE_MYSQLSUPPORT=ON \
      -DENABLE_INTERNALMYSQL=ON \
      -DENABLE_MEDIAPLAYER=ON \
      -DENABLE_DBUS=ON \
      -DENABLE_APPSTYLES=ON \
      -DENABLE_QWEBENGINE=OFF \
      -Wno-dev \
      ..

scan-build -o $ORIG_WD \
           -no-failure-reports \
           -analyze-headers \
           --keep-empty \
           --html-title $TITLE \
           -v \
           -k \
           make -j4

