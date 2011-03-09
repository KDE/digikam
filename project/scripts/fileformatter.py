#!/usr/bin/env python


# this scripts depends on Python 2.7 or higher

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
    args.append("--brackets=break")
    args.append("--indent=spaces=4")
    args.append("--convert-tabs")
    args.append("--indent-switches")
    args.append("--break-blocks")
    args.append("--break-closing-brackets")
    args.append("--pad-header")
    args.append("--align-pointer=type")
    args.append("--indent-col1-comments")
    args.append("--add-brackets")
    args.append("--min-conditional-indent=0")
    args.append(f)

    if (verbose):
        process = subprocess.Popen(args)
    else:
        process = subprocess.Popen(args, stdout=subprocess.PIPE)
    process.communicate()
    return process.returncode

# ---------------------------------------------------

def get_files(path, file_ext):
    files2check = [os.path.join(r,f) for r,dirs,files in os.walk(path) for f in
            files if os.path.splitext(os.path.join(r,f))[1] in file_ext]

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
    path = args.path
    verbose = args.verbose
    backup = args.backup

    errors = format_path(args.path, args.verbose)

    if (args.backup == False):
        errors += cleanup_path(args.path, args.verbose)
        
    return errors

# ---------------------------------------------------

if __name__ == "__main__":
    sys.exit(main())

