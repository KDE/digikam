#!/usr/bin/env python

# ============================================================
# 
# This file is a part of digiKam project
# http://www.digikam.org
# 
# Date        : 2011-03-09
# Description : a helper script for formatting the digiKam source code
# 
# Copyright (C) 2011 by Andi Clemens <andi dot clemens at gmail dot com>
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
#     AStyle:               http://astyle.sourceforge.net




import os
import sys
import subprocess

# ---------------------------------------------------

# if neccessary, enter the absolute path to the astyle executable
astyle = "astyle"

# the file type that should be formatted (defined by its extension)
file_srcs = [
        ".cpp",
        ".c",
        ".hpp",
        ".h",
]

# the file type that defines a backup file (defined by its extension)
file_backup = [
        ".orig",
]

# ---------------------------------------------------

def format_file(f, verbose=False):
    args = list()
    args.append(astyle)
    args.append("--mode=c")
    args.append("--brackets=break")
    args.append("--indent=spaces=4")
    args.append("--convert-tabs")
    args.append("--indent-switches")
    args.append("--break-blocks")
    args.append("--break-closing-brackets")
    args.append("--pad-oper")
    args.append("--pad-header")
    args.append("--unpad-paren")
    args.append("--align-pointer=type")
    args.append("--align-reference=type")
    args.append("--indent-col1-comments")
    args.append("--add-brackets")
    args.append("--min-conditional-indent=0")
    args.append("--max-instatement-indent=70")
    args.append(f)

    if (verbose):
        process = subprocess.Popen(args)
    else:
        process = subprocess.Popen(args, stdout=subprocess.PIPE)
    process.communicate()
    return process.returncode

# ---------------------------------------------------

def get_files(path, file_ext):
    files2check = set()

    if os.path.isfile(path):
        for ext in file_ext:
            if ext in file_backup:
                if os.path.isfile(path + ext):
                    files2check.add(path + ext)
            elif os.path.isfile(path):
                files2check.add(path)
    elif os.path.isdir(path):
        files2check.update([os.path.join(r,f) for r,dirs,files in os.walk(path) for f in
                files if os.path.splitext(os.path.join(r,f))[1] in file_ext])
    return files2check

# ---------------------------------------------------

def format_path(path, verbose=False):
    errors = 0

    files2check = get_files(path, file_srcs)

    for f in files2check:
        if format_file(f, verbose) != 0:
            print("An error occured while formatting '%s'" % f)
            errors += 1

    if (verbose):
        print("Formatted %d source files..." % (len(files2check) - errors))

    return errors 

# ---------------------------------------------------

def cleanup_path(path, verbose):
	# TODO: error handling
    errors = 0
    
    files2check = get_files(path, file_backup)

    for f in files2check:
        os.remove(f)
        if (verbose):
            print("removing %s" % f)
    if (verbose):
        print("Removed %d backup files..." % (len(files2check) - errors))

    return errors

# ---------------------------------------------------

def main():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("path", help="the path where the source files are located")
    parser.add_argument("-v", "--verbose", help="verbose output", default=False, action="store_true")
    parser.add_argument("-b", "--backup", help="keep backup files", default=False, action="store_true")
    args = parser.parse_args()

    errors = format_path(args.path, args.verbose)

    if (args.backup == False):
        errors += cleanup_path(args.path, args.verbose)
        
    return errors

# ---------------------------------------------------

if __name__ == "__main__":
    sys.exit(main())

