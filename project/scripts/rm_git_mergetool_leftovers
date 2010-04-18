#!/bin/sh

# Use this script to clean up your working dir before building the API documentation

E_ERROR=1

function usage() {
    echo "Usage: $(basename $0) <path>"
}

if [ $# -ne 1 ]; then
    usage
    exit $E_ERROR
fi

if [ -d $1 ]; then
    find $1 -type f -name "*BASE*" \
        -or -name "*REMOTE*" \
        -or -name "*LOCAL*" \
        -or -name "*.orig" \
        -exec rm {} \;
    exit $?
else
    echo "'$1' is not a valid directory. Make sure you have permissions to access this location."
    exit $E_ERROR
fi

