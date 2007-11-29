#! /bin/sh
$PREPARETIPS > tips.cpp
$EXTRACTRC `find . -name \*.rc` >> rc.cpp || exit 11
$EXTRACTRC `find . -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.h -o -name \*.cpp` -o $podir/digikam.pot
rm -f tips.cpp
rm -f rc.cpp
