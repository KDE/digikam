#! /bin/sh
(cd data && $PREPARETIPS > ../tips.cpp)
$EXTRACTRC `find . -name \*.rc -o -name \*.ui` >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.h -o -name \*.cpp | grep -v '/tests/'` `find digikam -name \*.h.cmake` -o $podir/digikam.pot
rm -f tips.cpp
rm -f rc.cpp
