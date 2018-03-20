#! /usr/bin/python

# Script to list recursive dylib dependencies for binaries/dylibs passed as arguments.
# Modified from https://github.com/mixxxdj/mixxx/blob/master/build/osx/otool.py.
#
# Copyright (c) 2015,      Shanti, <listaccount at revenant dot org>
# Copyright (c) 2015-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

import sys
import os
import re
import argparse

# ---------------------------------------------------------------

def system(s):
    "wrap system() to give us feedback on what it's doing"
    "anything using this call should be fixed to use SCons's declarative style (once you figure that out, right nick?)"
    print s,
    sys.stdout.flush() # ignore line buffering.
    result = os.system(s)
    print
    return result

# ---------------------------------------------------------------

SYSTEM_FRAMEWORKS = ["/System/Library/Frameworks"]
SYSTEM_LIBPATH    = ["/usr/lib"] # anything else?

# paths to libs that we should copy in
LOCAL_FRAMEWORKS  = [
    os.path.expanduser("~/Library/Frameworks"),
                       "/Library/Frameworks",
                       "/Network/Library/Frameworks"
]

LOCAL_LIBPATH = filter(lambda x:
    os.path.isdir(x),
    ["/usr/local/lib",
     "/opt/local/lib",
     "/sw/local/lib"]
)

# However
FRAMEWORKS = LOCAL_FRAMEWORKS + SYSTEM_FRAMEWORKS
LIBPATH    = LOCAL_LIBPATH + SYSTEM_LIBPATH

# ---------------------------------------------------------------

# otool parsing
def otool(binary):

    "return in a list of strings the OS X 'install names' of Mach-O binaries (dylibs and programs)"
    "Do not run this on object code archive (.a) files, it is not designed for that."
    #if not os.path.exists(binary): raise Exception("'%s' not found." % binary)

    if not type(binary) == str: raise ValueError("otool() requires a path (as a string)")

    stdin, stdout, stderr = os.popen3('otool -L "%s"' % binary)

    try:
        # discard the first line since it is just the name of the file or an error message (or if reading .as, the first item on the list)
        header = stdout.readline()

        if not binary+":\n" == header:

            # as far as I know otool -L only parses .dylibs and .a (but if it does anything else we should cover that case here)
            if header.startswith("Archive : "):
                raise Exception("'%s' an archive (.a) file." % binary)
            else:
                raise Exception(stderr.readline().strip())

        def parse(l):
            return l[1:l.rfind("(")-1]

        return [parse(l[:-1]) for l in stdout] #[:-1] is for stripping the trailing \n

    finally:
        stdin.close()
        stdout.close()
        stderr.close()

# ---------------------------------------------------------------

def dependencies(binary):
    l = otool(binary)

    if os.path.basename(l[0]) == os.path.basename(binary):
        id = l.pop(0)
        #print "Removing -id field result %s from %s" % (id, binary)
    return l

# ---------------------------------------------------------------

def embed_dependencies(binary,
                       # Defaults from
                       # http://www.kernelthread.com/mac/osx/programming.html
                       LOCAL = [os.path.expanduser("~/Library/Frameworks"),
                                                   "/Library/Frameworks",
                                                   "/Network/Library/Frameworks",
                                                   "/usr/local/lib",
                                                   "/opt/local/lib",
                                                   "/sw/local/lib"],
                       SYSTEM = ["/System/Library/Frameworks",
                                 "/Network/Library/Frameworks",
                                 "/usr/lib"]):
    "LOCAL: a list of paths to consider to be for installed libs that must therefore be bundled. Override this if you are not getting the right libs."
    "SYSTEM: a list of system library search paths that should be searched for system libs but should not be recursed into; this is needed. Override this if you want to bunde the system libs for some reason." #XXX it's useful to expose LOCAL but is this really needed?
    "this is a badly named function"
    "Note: sometimes Mach-O binaries depend on themselves. Deal with it."

    # "ignore_missing means whether to ignore if we can't load a binary for examination
    # (e.g. if you have references to plugins) XXX is the list"

    #binary = os.path.abspath(binary)

    todo = dependencies(binary)
    done = []
    orig = []

    # This code can be factored.

    while todo:

        # because of how this is written, popping from the end is a depth-first search. 
        # Popping from the front would be a breadth-first search. Neat!

        e = todo.pop()

        # Figure out the absolute path to the library

        if e.startswith('/'):
            p = e
        elif e.startswith('@'):
            # it's a relative load path
            raise Exception("Unable to make heads nor tails, sah, of install name '%s'. Relative paths are for already-bundled binaries, this function does not support them." % e)
        else:
            # experiments show that giving an unspecified path is asking dyld(1) to find the library for us. This covers that case.

            for P in ['']+LOCAL+SYSTEM: 
                p = os.path.abspath(os.path.join(P, e))
                #print "SEARCHING IN LIBPATH; TRYING", p

                if os.path.exists(p):
                    break
            else:
                p = e # fallthrough to the exception below #XXX icky bad logic, there must be a way to avoid saying exists() twice

        if not os.path.exists(p):
            raise Exception("Dependent library '%s' not found. Make sure it is installed." % e)

        if not any(p.startswith(P) for P in SYSTEM):

            if ".framework/Versions/" in p:
                # If dependency is a framework, return
                # framework directory not shared library

                f = re.sub("/Versions/[0-9]*/.*","",p)

                if f not in done:
                    done.append(f)
                    todo.extend(dependencies(p))
                    orig.append(e)
            elif p not in done:
                done.append(p)
                todo.extend(dependencies(p))
                orig.append(e)

    assert all(e.startswith("/") for e in done), "embed_dependencies() is broken, some path in this list is not absolute: %s" % (done,)

    return sorted(done)

# ---------------------------------------------------------------

deplist = []

for dep in sys.argv[1:]:
    deplist = deplist + embed_dependencies(dep)

depset = set(deplist)

for dep in depset:
   print dep
