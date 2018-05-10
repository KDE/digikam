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

cd $ORIG_DIR

SCANBUILD_DIR=$(find ${REPORT_DIR} -maxdepth 1 -not -empty -not -name `basename ${REPORT_DIR}`)
echo "Clang Report $TITLE to publish is located to $SCANBUILD_DIR"

# Remove unwanted lines in report accordingly with Krazy configuration.
# Note: Clang do not have an option to ignore directories to scan at compilation time.
# Se this entry in Clang bugzilla: https://bugs.llvm.org/show_bug.cgi?id=22594
krazySkipConfig

for DROP_ITEM in $KRAZY_FILTERS ; do
    echo -e "--- drop $DROP_ITEM from index.html with statistics adjustements"

    # List all report types including current pattern to drop.
    REPORT_ENTRIES=( $(grep $DROP_ITEM $SCANBUILD_DIR/index.html) )

    # STAT_ENTRIES array contains the multi-entries list of statistic types to remove.
    STAT_ENTRIES=()   # clear array

    for RITEM in "${REPORT_ENTRIES[@]}" ; do
        if [[ $RITEM = *bt_* ]]; then
            STAT_ENTRIES[${#STAT_ENTRIES[*]}]=$(echo $RITEM | awk -F "\"" '{print $2}')
        fi
    done

    # to update total statistic with current pattern to drop
    TOTAL_COUNT=0

    # update report statistics values.
    for SITEM in "${STAT_ENTRIES[@]}" ; do
        ORG_STAT_LINE=$(grep "ToggleDisplay(this," $SCANBUILD_DIR/index.html | grep "${SITEM}")
        STAT_VAL=$(echo $ORG_STAT_LINE | grep -o -P '(?<=class=\"Q\">).*(?=<\/td><td>)')
        ORG_STR="class=\"Q\">$STAT_VAL<\/td><td>"
        STAT_VAL=$((STAT_VAL-1))
        NEW_STR="class=\"Q\">$STAT_VAL<\/td><td>"
        NEW_STAT_LINE=${ORG_STAT_LINE/$ORG_STR/$NEW_STR}
        sed -i "s|$ORG_STAT_LINE|$NEW_STAT_LINE|" $SCANBUILD_DIR/index.html
        TOTAL_COUNT=$((TOTAL_COUNT+1))
    done

    # decrease total statistics with current TOTOAL_COUNT

    TOTAL_ORG_STAT_LINE=$(grep "CopyCheckedStateToCheckButtons(this)" $SCANBUILD_DIR/index.html | grep "AllBugsCheck")
    TOTAL_STAT_VAL=$(echo $TOTAL_ORG_STAT_LINE | grep -o -P '(?<=class=\"Q\">).*(?=<\/td><td>)')
    TOTAL_ORG_STR="class=\"Q\">$TOTAL_STAT_VAL<\/td><td>"
    TOTAL_STAT_VAL=$((TOTAL_STAT_VAL-TOTAL_COUNT))
    TOTAL_NEW_STR="class=\"Q\">$TOTAL_STAT_VAL<\/td><td>"
    TOTAL_NEW_STAT_LINE=${TOTAL_ORG_STAT_LINE/$TOTAL_ORG_STR/$TOTAL_NEW_STR}
    sed -i "s|$TOTAL_ORG_STAT_LINE|$TOTAL_NEW_STAT_LINE|" $SCANBUILD_DIR/index.html

    # Remove the lines including current pattern to drop.
    grep -v "$DROP_ITEM" $SCANBUILD_DIR/index.html > $SCANBUILD_DIR/temp && mv $SCANBUILD_DIR/temp $SCANBUILD_DIR/index.html
done

# update www.digikam.org report section.
updateReportToWebsite "clang" $SCANBUILD_DIR $TITLE

cd $ORIG_DIR
