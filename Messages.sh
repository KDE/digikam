#! /bin/sh
$EXTRACTRC `find . -name \*.rc -o -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.h -o -name \*.cpp | grep -v '/tests/'` `find app -name \*.h.cmake.in` -o $podir/digikam.pot
rm -f rc.cpp
