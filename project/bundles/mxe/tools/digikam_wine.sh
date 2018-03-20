#!/bin/sh

export MYDK="`pwd`\\bundle"
export KDEHOME=$MYDK
#export KDESYCOCA=$MYDK\\kbuildsycoca5
export XDG_DATA_DIRS=$MYDK\\share\\xdg\\
export KDEDIRS=$MYDK
export KDEDIR=$MYDK
export WINEPATH=$MYDK\\:$WINEPATH

cd ../bundle

wine64 kbuildsycoca5
wine64 digikam

