#!/bin/bash

username=$(whoami)
databasedir=$(cat $(kde4-config --localprefix)share/config/digikamrc | grep "Database File Path" | sed -e 's/Database File Path=//')
proc="$(ps aux | grep $username | grep -v $0 | grep -w digikam | grep -v grep)"
if [ "$proc" != "" ]
then
    echo "Please make sure to shutdown digiKam first."
    echo "Cleaning up the database while the program is running might not be as safe as if the application has been shut down."
    exit 1
fi

curdir=$(pwd)

cd $databasedir
if [ $? == 0 ]
then
    echo "Cleaning up databases in $(pwd)"
    for F in $(find . -type f -name 'digikam*.db' -print)
    do
        echo -n "$F ... "
        sqlite3 $F "VACUUM;"
        if [ $? == 0 ]
        then
            echo "done"
        else
            echo "sqlite3 was not able to cleanup the database."
        fi
    done
else
    echo -e "\n !!!! Something went terrible wrong here.... I was not able to enter the database folder.\n"
fi
echo "Job finished";

cd $curdir
