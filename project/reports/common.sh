#!/bin/bash

# Copyright (c) 2013-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

########################################################################
# checks if branch has something pending
function parseGitDirty()
{
    git diff --quiet --ignore-submodules HEAD 2>/dev/null; [ $? -eq 1 ] && echo "M"
}

########################################################################
# gets the current git branch
function parseGitBranch()
{
    git branch --no-color 2> /dev/null | sed -e '/^[^*]/d' -e "s/* \(.*\)/\1/"
}

########################################################################
# get last commit hash prepended with @ (i.e. @8a323d0)
function parseGitHash()
{
    git rev-parse --short HEAD 2> /dev/null | sed "s/\(.*\)/-rev-\1/"
}

########################################################################
# Update www.digikam.org static analyze report section.
# arg1: static analyzer name (clang, cppcheck, krazy, ...).
# arg2: static analyzer report directory with html contents.
# arg3: static analyzer report title.
#
function updateReportToWebsite()
{
    WEBSITE_DIR="`pwd`/site"

    rm -fr $WEBSITE_DIR

    git clone git@git.kde.org:websites/digikam-org $WEBSITE_DIR

    cd $WEBSITE_DIR

    git checkout -b dev remotes/origin/dev

    rm -r $WEBSITE_DIR/static/reports/$1/*
    mkdir $WEBSITE_DIR/static/reports/$1
    cp -r $2/* $WEBSITE_DIR/static/reports/$1/

    # Add new report contents in dev branch

    git add $WEBSITE_DIR/static/reports/$1/*
    git commit . -m"update $1 static analyzer report $3."
    git push

    # update master branch

    git checkout master
    git merge dev -m"Update $1 static analyzer report $3.
See https://www.digikam.org/reports/$1 for details.
CCMAIL: digikam-bugs-null@kde.org"
    git push

    echo "$1 Report $3 published to https://www.digikam.org/reports/$1"
    echo "Web site will be synchronized in few minutes..."
}

########################################################################
# Extract Krazy configuration about unwanted directory to parse with static analyzer
function krazySkipConfig()
{
    KRAZY_FILE="`pwd`/../../.krazy"
    KRAZY_FILTERS=$(sed -n '/SKIP/p' ${KRAZY_FILE} | sed -e 's/SKIP //g' | sed -e 's/|/\n/g' | sed 's/^.\(.*\).$/\1/')
    echo "Directories to ignore from Krazy configuration: $KRAZY_FILTERS"
}

########################################################################
# Check CPU core available (Linux or MacOS)
function checksCPUCores()
{
    CPU_CORES=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu)

    if [[ $CPU_CORES -gt 1 ]]; then
        CPU_CORES=$((CPU_CORES-1))
    fi

    echo "CPU Cores to use : $CPU_CORES"
}
