#!/usr/bin/env python

# ============================================================
# 
# This file is a part of digiKam project
# http://www.digikam.org
# 
# Date        : 2009-10-12
# Description : a helper script for formatting the digiKam source code
# 
# Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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


import os
import sys
import re

# --------------------------------------------------
# VARIABLES
sourcedir    = ""
patternsdir  = os.path.dirname(os.path.realpath(__file__))
patternsurl  = os.path.join(patternsdir, "known_patterns.txt")
filelist     = list()

# PATTERNS
pfile       = re.compile("^.*\.(cpp)$")                   # filename pattern
pic         = re.compile("^\s*\/\/\s*.*includes?\.*\s*$") # old include comments
pch         = re.compile(".*={10,}.*")                    # copyright header boundries
plocal      = re.compile("\"[a-zA-Z].*\.h\"")             # local includes
pqt         = re.compile("<[Qq].*(.h)?>")                 # Qt includes
pkde        = re.compile("<k.*\.h>")                      # KDE includes
pcpp        = re.compile("<c.*(.h)?>")                    # C++ includes
pinclude    = "#include"                                  # include statement
pknown      = dict()                                      # will hold known patterns


# FIXME: maybe it is better to generate all valid header files from filesystem
# paths instead of those regex patterns

# --------------------------------------------------
def intro(dir, pattern):
    print()
    print("digiKam code cleaner")
    print()
    print("SOURCEDIR:\t%s" % os.path.realpath(dir))
    print("PATTERNS FILE:\t%s" % pattern)
    print()

# --------------------------------------------------
def legend():
    print()
    print("-" * 50)
    print()
    print("Legend:")
    print()
    print("PARSE ERROR")
    print("\tThe sourcecode is malformed, the shown lines need to be checked manually")
    print()
    print("MISSING / WRONG REGEX")
    print("\tThe pattern for an include statement was not recognized.")
    print("\tAdd it to the patterns file or create an regex for it.")
    print("\tFor now, the patterns can be found under '// OTHERS includes.'")
    print("\tin the sourceode.")
    print()

# --------------------------------------------------
# generate known patterns map
def generate_known_patterns(file):
    patterns = dict()
    try:
        f = open(file, 'r')
        for line in f:
            k, v = line.strip().split()
            if not k in patterns:
                patterns[k] = list()
            patterns[k].append(v)
    except:
        patterns = dict()
    finally:
        f.close()

    return patterns

# --------------------------------------------------
# generate a sorted includes list
def generate_includes(fp, includes, comment, bstart=None, bend=None):
    tmp = list()
    if (includes):
        includes.sort()
        if (comment):
            fp.write("\n// %s includes\n\n" % comment)
        else:
            fp.write("\n")
        if bstart: fp.write(bstart+"\n")
        for inc in includes: fp.write("#include " + inc)
        if bend: fp.write(bend+"\n")

# --------------------------------------------------
# strip the whitespace / empty lines from the sourcecode block
def strip_content(content):
    return "".join(str(c) for c in content).strip()

# --------------------------------------------------
# parse the files in filelist and generate a new sourcefile
def parse_files(filelist):
    errors = False

    print("parsing %s files... " % len(filelist))
    print()

    for file in filelist:
        inheader    = False
        i = 0
        nameonly = '%s' % file.split(".")[-2].split("/")[-1]
        pow = re.compile("\"%s\.(h|moc)\"" % nameonly)
        
        header      = list()
        includes    = list()
        owninc      = list()
        localinc    = list()
        qtinc       = list()
        kdeinc      = list()
        cppinc      = list()
        cinc        = list()
        sourcecode  = list()

        f = open(file, 'r')

        for line in f:
            i += 1

            if pinclude in line:
                try:
                    command, include = line.split(" ", 1)
                except:
                    errors = True
                    print("PARSE ERROR: %s (%s)" % (line.strip(), file))
                    continue

                if pow.match(include):                   owninc.append(include)
                elif (include.strip() in pknown['kde']): kdeinc.append(include)
                elif (include.strip() in pknown['cpp']): cppinc.append(include)
                elif (include.strip() in pknown['c']):   cinc.append(include)

                elif plocal.match(include):     localinc.append(include)
                elif pqt.match(include):        qtinc.append(include)
                elif pkde.match(include):       kdeinc.append(include)
                elif pcpp.match(include):       cppinc.append(include)

                else:
                    includes.append(include)
                    errors = True
                    print("MISSING / WRONG REGEX: %s (%s)" % (line.strip(), file))

            elif pic.match(line): continue

            elif pch.match(line):
                if i < 5 and not inheader:
                    inheader = True
                    header.append(line)
                elif inheader:
                    inheader = False
                    header.append(line)

            elif inheader:
                header.append(line)
                
            else: sourcecode.append(line)

            
        f.close()

        #output
        fw = open(file, 'w')
        for h in header: fw.write(h)

        # order is important!!
        generate_includes(fw, owninc,     "")
        generate_includes(fw, includes,   "OTHERS")
        generate_includes(fw, cinc,       "C ANSI", 'extern "C"\n{', '}')
        generate_includes(fw, cppinc,     "C++")
        generate_includes(fw, qtinc,      "Qt")
        generate_includes(fw, kdeinc,     "KDE")
        generate_includes(fw, localinc,   "Local")

        fw.write("\n"+strip_content(sourcecode)+"\n");
        fw.close()
    return errors

# --------------------------------------------------
# MAIN
if __name__ == '__main__':

    # check for valid arguments first
    if (len(sys.argv) != 2):
        print("usage: cleanup_headers.py <sourcedir>")
        sys.exit(1)

    sourcedir = sys.argv[1]
    
    intro(sourcedir, patternsurl)

    # scan given source dir
    filelist = [os.path.join(root, file) for root, dirs, files in os.walk(sourcedir) for file in files if pfile.match(file)]

    # read known patterns
    pknown = generate_known_patterns(patternsurl)

    # parse files
    error = parse_files(filelist)
    if error: legend()


