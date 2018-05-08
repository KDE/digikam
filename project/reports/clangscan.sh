#!/bin/bash

# Copyright (c) 2013-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Run Clang static analyzer on whole digiKam source code.
# https://clang-analyzer.llvm.org/
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

. ./common.sh

checksCPUCores

ORIG_WD="`pwd`"
REPORT_DIR="${ORIG_WD}/report.scan"
WEBSITE_DIR="${ORIG_WD}/site"

# Get active git branches to create report description string
TITLE="digiKam-$(parseGitBranch)$(parseGitHash)"
echo "Clang Static Analyzer task name: $TITLE"

# Clean up and prepare to scan.

rm -fr $REPORT_DIR
rm -fr $WEBSITE_DIR

cd ../..

rm -fr build.scan
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

scan-build -o $REPORT_DIR \
           -no-failure-reports \
           --keep-empty \
           --html-title $TITLE \
           -v \
           -k \
           make -j$CPU_CORES

SCANBUILD_DIR=$(find ${REPORT_DIR} -maxdepth 1 -not -empty -not -name `basename ${REPORT_DIR}`)
echo "Clang Report $TITLE to publish is located to $SCANBUILD_DIR"

# Remove unwanted lines in report accordingly with Krazy configuration.
# Note: Clang do not have an option to ignore directories to scan at compilation time.
krazySkipConfig

for DROP_ITEM in $KRAZY_FILTERS ; do
    echo "drop $DROP_ITEM from $SCANBUILD_DIR/index.html"
    grep -v "$DROP_ITEM" $SCANBUILD_DIR/index.html > $SCANBUILD_DIR/temp && mv $SCANBUILD_DIR/temp $SCANBUILD_DIR/index.html
done

# update www.digikam.org report section.

git clone git@git.kde.org:websites/digikam-org $WEBSITE_DIR

cd $WEBSITE_DIR

git checkout -b dev remotes/origin/dev

rm -r $WEBSITE_DIR/static/reports/clang/*
cp -r $SCANBUILD_DIR/* $WEBSITE_DIR/static/reports/clang/

# Add new report contents in dev branch

git add $WEBSITE_DIR/static/reports/clang/*
git commit . -m"update Clang static analyzer report $TITLE. See https://www.digikam.org/reports/clang for details."
git push

# update master branch

git checkout master
git merge dev -m"update Clang static analyzer report $TITLE. See https://www.digikam.org/reports/clang for details."
git push

cd $ORIG_DIR

echo "Clang Report $TITLE published to https://www.digikam.org/reports/clang"
echo "Web site will be synchronized in few minutes..."
