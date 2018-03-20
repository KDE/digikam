#! /bin/sh
$EXTRACTRC `find ./core -name \*.rc -o -name \*.ui | grep -v '/tests/'` >> rc.cpp || exit 11
$XGETTEXT `find ./core -name \*.h -o -name \*.cpp | grep -v '/tests/'` `find ./core/app -name \*.h.cmake.in` -o $podir/digikam.pot
rm -f rc.cpp
