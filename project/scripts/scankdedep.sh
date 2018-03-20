#!/bin/sh

grep --exclude-dir PORT.KF5 -r "#include <k" * | \
   grep -v "#include <klocalizedstring.h>" | \
   grep -v "config"                        | \
   grep -v "ipi"                           | \
   grep -v "action"
