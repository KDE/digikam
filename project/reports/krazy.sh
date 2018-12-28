#!/bin/bash

# Copyright (c) 2013-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Run Krazy static analyzer on whole digiKam source code.
# https://github.com/Krazy-collection/krazy
# Dependencies:
#  - Perl:Tie::IxHash and Perl:XML::LibXML modules at run-time.
#  - Saxon java xml parser to export report as HTML.
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

# Compute static analyzer output as XML
TITLE_EXT=$TITLE+"
This is the static analyzis processed with Extra checkers.
See <a href=\"http://ebn.kde.org/krazy/reports/extragear/graphics/digikam/\">EBN</a> for standard analyzis."

krazy2all --export xml \
          --title $TITLE \
          --no-brief \
          --strict all \
          --priority all \
          --verbose \
          --topdir ../../ \
          --outfile ./report.krazy.xml

# Clean up XML file

sed -i "s/repo-rev value=\"unknown\"/repo-rev value=\"$(parseGitBranch)$(parseGitHash)\"/g" ./report.krazy.xml

DROP_PATH=$(echo $ORIG_WD | rev | cut -d'/' -f3- | rev | sed 's_/_\\/_g')
sed -i "s/$DROP_PATH//g" ./report.krazy.xml

mkdir -p $REPORT_DIR

# Process XML file to generate HTML

java -jar /usr/share/java/saxon/saxon.jar \
     -o:$REPORT_DIR/index.html \
     -im:krazy2ebn \
     ./report.krazy.xml \
     ./krazy/krazy-main.xsl \
     module=graphics \
     submodule=digikam \
     component=extragear

cp ./krazy/style.css $REPORT_DIR/

# update www.digikam.org report section.
updateReportToWebsite "krazy" $REPORT_DIR $TITLE $(parseGitBranch)

cd $ORIG_DIR
