#!/usr/bin/env python

# A simple script that extracts config group entry names from a file,
# generates variable definitions for d-classes and replaces the strings of
# these entries with the defined variables. The result is stored in a new file.
# Expects a file name as argument
#
# author: Johannes Wienke <languitar at semipol dot de>

import sys
import re
import os

filename = sys.argv[1]

print("Opening file " + filename)

source = open(filename, "r")
lines = source.readlines()

expression = re.compile("readEntry\(\"([\s\w]+)\",")

configKeys = []

def makeStringDefinition(key):
    return '"' + key + '"'

def makeVariable(key):
    key = key.replace(" ", "")
    return "config" + key[0].capitalize() + key[1:] + "Entry";

for line in lines:
    match = expression.search(line)
    
    if match == None:
        continue
    
    configKey = match.group(1)
    configKeys.append(configKey)
    
# initialization
print("init:")
for key in configKeys:
    print("\t" + makeVariable(key) + " = " + makeStringDefinition(key) + ";")
    
print("")

# definition
print("def:")
for key in configKeys:
    print("\tQString " + makeVariable(key) + ";")
   
# generate output file
outname = filename + ".out"
print("writing replaced entries to " + outname)

source.seek(0)
contents = source.read()

for key in configKeys:
    print("replacing " + makeStringDefinition(key) + " with d->" + makeVariable(key))
    contents = contents.replace(makeStringDefinition(key), "d->" + makeVariable(key))
    
out = open(outname, "w")
out.write(contents)
out.close()

source.close()
