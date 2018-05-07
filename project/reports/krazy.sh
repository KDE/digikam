#!/bin/bash

# Copyright (c) 2013-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Run Krazy static analyzer on whole digiKam source code.
# https://github.com/Krazy-collection/krazy
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

. ./common.sh

checksCPUCores

ORIG_WD="`pwd`"
REPORT_DIR="${ORIG_WD}/report.krazy"
WEBSITE_DIR="${ORIG_WD}/site"

# Get active git branches to create report description string
TITLE="digiKam-$(parseGitBranch)$(parseGitHash)"
echo "Krazy Static Analyzer task name: $TITLE"

rm -fr $REPORT_DIR
rm -fr $WEBSITE_DIR
if [ ] ; then
krazy2all --export xml \
          --title $TITLE \
          --no-brief \
          --strict all \
          --priority all \
          --verbose \
          --topdir ../../ \
          --check multiclasses \
          --outfile ./report.krazy.xml
fi
# TODO : clean up xml file

mkdir -p $REPORT_DIR

java -jar /usr/share/java/saxon/saxon.jar \
     -o:$REPORT_DIR/index.html \
     -im:krazy2ebn \
     ./report.krazy.xml \
     ./krazy-main.xsl \
     module=graphics \
     submodule=digikam \
     component=extragear

cp ./style.css $REPORT_DIR/

exit

# update www.digikam.org report section.

git clone git@git.kde.org:websites/digikam-org $WEBSITE_DIR

cd $WEBSITE_DIR

git checkout -b dev remotes/origin/dev

rm -r $WEBSITE_DIR/static/reports/krazy/*
cp -r $REPORT_DIR/* $WEBSITE_DIR/static/reports/krazy/

# Add new report contents in dev branch

git add $WEBSITE_DIR/static/reports/krazy/*
git commit . -m"update Krazy static analyzer report $TITLE. See https://www.digikam.org/reports/krazy for details."
git push

# update master branch

git checkout master
git merge dev -m"update Krazy static analyzer report $TITLE. See https://www.digikam.org/reports/krazy for details."
git push

cd $ORIG_DIR

echo "Krazy Report $TITLE published to https://www.digikam.org/reports/krazy"
echo "Web site will be synchronized in few minutes..."
