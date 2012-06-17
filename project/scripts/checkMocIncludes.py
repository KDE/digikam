#! /usr/bin/env python
# encoding: utf-8


import os


def isMocFileUser(path, f):
    if not f.endswith(".h"):
        return False

    with open(os.path.join(root,f), "r") as fp:
        for line in fp:
            if "Q_OBJECT" in line:
                return True
    return False


def checkMocFile(path, f):
    inFilename = os.path.join(path,f)
    inPath, inExt = os.path.splitext(inFilename)

    try:
        with open(inPath + ".cpp", "r") as fp:
            for line in fp: 
                if ".moc\"" in line:
                    return True
    except IOError:
        # there is no .cpp file, so return success
        return True
    return False


if __name__ == "__main__":
    for root,dirs,files in os.walk("."):
        for f in files:
            if isMocFileUser(root,f):
                if not checkMocFile(root,f):
                    print("no moc include: '%s'" % os.path.join(root,f))

