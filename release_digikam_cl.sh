#!/bin/sh

# script to generate a changelog file
# (maybe integrate into release script?)
#
# (c) 2006 Achim Bohnet <ach@mpe.mpg.de>
# Some parts of this code taken from release_digikam.rb,
# idea from Tom Albers
# License: GNU General Public License V2


name="digikam"
egmodule="graphics"
startrev="606700"   # check digikam/ChangeLog for last rev/date. (add 1 to last rev)
#startrev='{2006-10-15}'   # check existing ChangeLog file for the most recent entry

svnbase="svn+ssh://toma@svn.kde.org/home/kde"
svnbase="https://ach@svn.kde.org/home/kde/"
svnroot="trunk"
adminroot="$svnbase/branches/KDE/3.5"

#----------------------------------------------------------------

set -x
export LC_ALL=en_US.UTF-8

# using accounts it with svn2log.py -u  add even more of email addresses to changelog :(
#svn cat $svnbase/trunk/KDE/kde-common/accounts > accounts.tmp.$$
svn cat $adminroot/kde-common/release/svn2log.py > svn2log.py.tmp-$$
svn log -v --xml -r HEAD:$startrev $svnbase/$svnroot/extragear/$egmodule/$name |
	python ./svn2log.py.tmp-$$ -p /$svnroot/extragear/$egmodule -o ChangeLog.new-entries
rm svn2log.py.tmp-$$
