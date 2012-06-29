#! /usr/bin/env python
# encoding: utf-8

# ============================================================
# 
# This file is a part of digiKam project
# http://www.digikam.org
# 
# Date        : 2012-06-29
# Description : a helper script for finding source code with no moc include
# 
# Copyright (C) 2012 by Andi Clemens <andi dot clemens at googlemail dot com>
# 
# This program is free software; you can redistribute it
# and/or modify it under the terms of the GNU General
# Public License as published by the Free Software Foundation;
# either version 2, or (at your option)
# any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# ============================================================ */


# prerequisites:
#     Python 2.7 or higher: http://www.python.org

import os
import re

def sourceFile(path, f):
    if not f.endswith(".h"):
        return None
    inFileName, inFileExt = os.path.splitext(f)
    cppFile = os.path.join(path, inFileName + ".cpp")
    return cppFile


def isMocFileUser(path, f):
    if not f.endswith(".h"):
        return False

    with open(os.path.join(path ,f), "r") as fp:
        for line in fp:
            if "Q_OBJECT" in line:
                return True
    return False


def checkForMocFileInclude(path, f):
    cppFile = sourceFile(path, f)
    if cppFile is None:
        return False

    inFileName, inFileExt = os.path.splitext(f)

    r = re.compile("""#include\\s+["<]%s.moc[">]""" % inFileName)

    try:
        with open(cppFile, "r") as fp:
            for line in fp: 
                if r.match(line):
                    return True
    except IOError:
        # there is no .cpp file, so return success
        return True
    return False


if __name__ == "__main__":
    for root,dirs,files in os.walk("."):
        for f in files:
            if isMocFileUser(root,f):
                if not checkForMocFileInclude(root,f):
                    cppFile = sourceFile(root, f)
                    if cppFile is None:
                        print("unable to find the source file: '%s'" % cppFile)
                    else:
                        print("no moc include: '%s'" % cppFile)

