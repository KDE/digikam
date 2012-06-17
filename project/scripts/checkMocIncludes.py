#! /usr/bin/env python
# encoding: utf-8


import os
import re


def isMocFileUser(path, f):
    if not f.endswith(".h"):
        return False

    with open(os.path.join(root,f), "r") as fp:
        for line in fp:
            if "Q_OBJECT" in line:
                return True
    return False


def checkForMocFileInclude(path, f):
    inFileName, inFileExt = os.path.splitext(f)
    cppFile = os.path.join(root,inFileName + ".cpp")

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
                    print("no moc include: '%s'" % os.path.join(root,f))

