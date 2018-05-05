#!/bin/bash

# Copyright (c) 2013-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Run Clang static analyzer on whole digiKam source code.
# https://clang-analyzer.llvm.org/
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

# --- Functions definitions --------------------------------

# checks if branch has something pending
function parseGitDirty()
{
    git diff --quiet --ignore-submodules HEAD 2>/dev/null; [ $? -eq 1 ] && echo "-M"
}

# gets the current git branch
function parseGitBranch()
{
    git branch --no-color 2> /dev/null | sed -e '/^[^*]/d' -e "s/* \(.*\)/\1$(parseGitDirty)/"
}

# get last commit hash prepended with @ (i.e. @8a323d0)
function parseGitHash()
{
    git rev-parse --short HEAD 2> /dev/null | sed "s/\(.*\)/rev-\1/"
}

# ----------------------------------------------------------

ORIG_WD="`pwd`"
REPORT_DIR="report.scan"

# Get active git branches to create report description string
TITLE="digiKam-$(parseGitBranch)$(parseGitHash)"
echo "Clang Static Analyzer task name: $TITLE"

CPU_CORES=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu)
echo "CPU cores detected to compile : $CPU_CORES."

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

scan-build -o $ORIG_WD/$REPORT_DIR \
           -no-failure-reports \
           --keep-empty \
           --html-title $TITLE \
           -v \
           -k \
           make -j$CPU_CORE

SCANBUILD_DIR=$(find ${ORIG_WD}/$REPORT_DIR -maxdepth 1 -not -empty -not -name `basename ${ORIG_WD}/$REPORT_DIR`)
echo $SCANBUILD_DIR

mv $SCANBUILD_DIR ./$TITLE
rm -fr $REPORT_DIR
