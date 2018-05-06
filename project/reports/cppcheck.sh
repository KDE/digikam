#!/bin/bash

# Copyright (c) 2013-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Run CppCheck static analyzer on whole digiKam source code.
# http://cppcheck.sourceforge.net/
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

. ./common.sh

checksCPUCores

ORIG_WD="`pwd`"
REPORT_DIR="${ORIG_WD}/report.cppcheck"
WEBSITE_DIR="${ORIG_WD}/site"

# Get active git branches to create report description string
TITLE="digiKam-$(parseGitBranch)$(parseGitHash)"
echo "CppCheck Static Analyzer task name: $TITLE"

rm -fr $REPORT_DIR
rm -fr $WEBSITE_DIR

cppcheck -j$CPU_CORES \
         --verbose \
         --xml \
         --platform=unix64 \
         --enable=warning \
         --report-progress \
         --suppress=*:*CImg.h* \
         -i../../core/libs/dimg/filters/greycstoration/cimg \
         -i../../core/libs/rawengine/libraw \
         -i../../core/libs/dngwriter/extra \
         -i../../core/libs/facesengine/dnnface \
         -i../../core/libs/facesengine/opencv3-face \
         -i../../core/libs/kmemoryinfo \
         -i../../core/libs/pgfutils/libpgf \
         -i../../core/libs/jpegutils/libjpeg \
         -i../../core/utilities/mediaserver/upnpsdk \
         -i../../core/utilities/assistants/webservices/common/o2 \
         ../../core \
         2> report.cppcheck.xml

cppcheck-htmlreport --file=report.cppcheck.xml \
                    --report-dir=$REPORT_DIR \
                    --source-dir=. \
                    --title=$TITLE

# update www.digikam.org report section.

git clone git@git.kde.org:websites/digikam-org $WEBSITE_DIR

cd $WEBSITE_DIR

git checkout -b dev remotes/origin/dev

rm -r $WEBSITE_DIR/static/reports/cppcheck/*
cp -r $REPORT_DIR/* $WEBSITE_DIR/static/reports/cppcheck

# Add new report contents in dev branch

git add $WEBSITE_DIR/static/reports/cppcheck/*
git commit . -m"update CppCheck static analyzer report $TITLE. See https://www.digikam.org/reports/cppcheck for details."
git push

# update master branch

git checkout master
git merge dev -m"update CppCheck static analyzer report $TITLE. See https://www.digikam.org/reports/cppcheck for details."
git push

cd $ORIG_DIR

echo "Clang Report $TITLE published to https://www.digikam.org/reports/cppcheck"
echo "Web site will be synchronized in few minutes..."

