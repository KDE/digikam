#!/bin/bash

# Copyright (c) 2013-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

ORIG_WD="`pwd`"
API_DIR="${ORIG_WD}/../../html"
WEBSITE_DIR="${ORIG_WD}/site"

rm -fr $API_DIR
rm -fr $WEBSITE_DIR

cd ${ORIG_WD}/../../ && doxygen
cd ${ORIG_WD}

git clone git@git.kde.org:websites/digikam-org $WEBSITE_DIR

cd $WEBSITE_DIR

git checkout -b dev remotes/origin/dev

rm -r $WEBSITE_DIR/static/api/*
cp -r $API_DIR/* $WEBSITE_DIR/static/api/

# Add new report contents in dev branch

git add $WEBSITE_DIR/static/api/*
git commit . -m"update API documentation."
git push

# update master branch

git checkout master
git merge dev -m"Update API documentation.
See https://www.digikam.org/api for details.
CCMAIL: digikam-bugs-null@kde.org"
git push

echo "API documentation published to https://www.digikam.org/api"
echo "Web site will be synchronized in few minutes..."
