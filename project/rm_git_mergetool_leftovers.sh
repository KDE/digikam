#!/bin/sh

# Use this script to clean up your working dir before building the API documentation

function usage() {
    echo "usage: " $0 "<path>"
}

if [ $# -ne 1 ]; then
    usage
    exit 0
fi

find $1 -type f -name "*BASE*" \
                -or -name "*REMOTE*" \
                -or -name "*LOCAL*" \
                -or -name "*.orig" \
        -exec rm {} \;
