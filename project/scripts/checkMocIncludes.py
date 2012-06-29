#! /usr/bin/env python
# encoding: utf-8


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

