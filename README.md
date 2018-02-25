digiKam - Professional Photo Management with the Power of Open Source

# About

digiKam is an advanced open-source digital photo management application that runs on Linux, Windows, and MacOS.
The application provides a comprehensive set of tools for importing, managing, editing, and sharing photos and RAW files.

[Albums View and Image Editor](https://c1.staticflickr.com/5/4216/35354951072_a034561b5e_c.jpg)

You can use digiKam's import capabilities to easily transfer photos, RAW files, and videos directly from your camera
and external storage devices (SD cards, USB disks, etc.). The application allows you to configure import settings
and rules that process and organize imported items on-the-fly.

digiKam organizes photos, RAW files, and videos into albums. But the application also features powerful tagging
tools that allow you to assign tags, ratings, and labels to photos and raw files. You can then use filtering
functionality to quickly find items that match specific criteria.

In addition to filtering functionality, digiKam features powerful searching capabilities that let you search
the photo library by a wide range of criteria. You can search photos by tags, labels, rating, data, location,
and even specific EXIF, IPTC, or XMP metadata. You can also combine several criteria for more advanced searches.
digiKam rely on Exiv2 library to handle metadata tag contents from files to populate the photo library.

digiKam can handle RAW files, and the application uses the excellent LibRaw library for decoding raw files.
The library is actively maintained and regularly updated to include support for the latest camera models.

The application provides a comprehensive set of editing tools. This includes basic tools for adjusting colors,
cropping, and sharpening as well as advanced tools for, curves adjustment, panorama stitching, and much more.
A special tool based on Lensfun library permit to apply lens corrections automatically on images.

Extended functionality in digiKam is implemented via a set of plugins. You can easily customize digiKam by enable
and disable individual tools, dedicated especially to import and export contents to remote web-services.

digiKam is based in part on the work of the Independent JPEG Group.

# Authors

See [AUTHORS](AUTHORS) file for details.

# Related URLs

http://www.digikam.org

# Contact

If you have questions, comments, suggestions to make do email at :

digikam-users@kde.org

If you want contribute to digiKam developments do email at :

digikam-devel@kde.org

IRC channel from irc.freenode.org server: #digikam

# Bug reports

IMPORTANT : the bug reports and wishlist entries are hosted by the KDE bug report
system which can be reached from the standard KDE help menu of digiKam.
A mail will automatically be sent to the digiKam development mailing list.
There is no need to contact directly the digiKam mailing list for a bug report
or a devel wish.

The current bugs and devel wishes reported to the bugzilla servers can be seen at these urls :

* [digiKam](http://bugs.kde.org/buglist.cgi?product=digikam&bug_status=UNCONFIRMED&bug_status=NEW&bug_status=ASSIGNED&bug_status=REOPENED)
* [Kipi-plugins](http://bugs.kde.org/buglist.cgi?product=kipiplugins&bug_status=UNCONFIRMED&bug_status=NEW&bug_status=ASSIGNED&bug_status=REOPENED)

Extra Bugzilla servers for shared libs used by digiKam :

* [GPhoto2 library](http://gphoto.org/bugs)
* [Exiv2 library](http://dev.exiv2.org/projects/exiv2/issues)
* [QtAV library](https://github.com/wang-bin/QtAV/issues)

# Dependencies

See the complete list of dependencies at this url:

https://cgit.kde.org/digikam-software-compilation.git/tree/DEPENDENCIES 

CMake compilation options to custom digiKam:

Use CMake `-DENABLE_KFILEMETADATASUPPORT=on`  flag to compile digiKam with KDE files indexer support                                 (disabled by default).
Use CMake `-DENABLE_AKONADICONTACTSUPPORT=on` flag to compile digiKam with KDE Mail Contacts support                                 (disabled by default).
Use CMake `-DENABLE_DBUS=off`                 flag to compile digiKam without DBus support                                           (enabled by default - for Linux desktop only).
Use CMake `-DENABLE_MEDIAPLAYER=on`           flag to compile digiKam with QtAV support                                              (disabled by default).
Use Cmake `-DENABLE_APPSTYLES=on`             flag to compile digiKam with widget application style support                          (disabled by default).
Use CMake `-DENABLE_OPENCV3=on`               flag to compile digiKam source code using OpenCV3 instead OpenCV2                      (disabled by default).

OpenCV3 support needs extra contrib modules package, especially 'face'
and 'legacy' components.

Mysql support options (experimental):

Use CMake `-DENABLE_MYSQLSUPPORT=on`          flag to compile digiKam with MysSql support                                            (disabled by default).
Use CMake `-DENABLE_INTERNALMYSQL=on`         flag to compile digiKam with internal MySQL server                                     (disabled by default).

Debug options:

Use CMake `-DBUILD_TESTING=on`                flag to compile digiKam source code unit tests                                         (disabled by default).

# Installation

In order to compile, just use something like that:

    export VERBOSE=1
    export QTDIR=/usr/lib/qt5
    export PATH=$QTDIR/bin:$PATH
    cmake .
    make
    sudo make install

Usual CMake options :

`-DCMAKE_INSTALL_PREFIX` : decide where the program will be install on your computer.
`-DCMAKE_BUILD_TYPE`     : decide which type of build you want. You can chose between:
                           `debugfull`.     : for hacking. Include all debug information.
                           `debug`.
                           `profile`.
                           `relwithdebinfo` : default. use gcc `-O2` `-g` options.
                           `release`        : generate stripped and optimized bin files. For packaging.

Compared to old autoconf options:

    cmake . -DCMAKE_BUILD_TYPE=debugfull" is equivalent to "./configure --enable-debug=full
    cmake . -DCMAKE_INSTALL_PREFIX=/usr"  is equivalent to "./configure --prefix=/usr

More details can be found at this url: http://techbase.kde.org/Development/Tutorials/CMake#Environment_Variables

Note: To know KDE install path on your computer, use `kf5-config --prefix` command line like this (with full debug object enabled):

    cmake . -DCMAKE_BUILD_TYPE=debugfull -DCMAKE_INSTALL_PREFIX=`kf5-config --prefix`

# Donate Money

If you love digiKam, you can help developers to buy new photo devices to test
and implement new features. Thanks in advance for your generous donations.

For more information, look at this url : https://www.digikam.org/donate/
