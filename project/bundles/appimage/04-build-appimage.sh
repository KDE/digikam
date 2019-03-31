#!/bin/bash

# Script to bundle data using previously-built KF5 with digiKam installation
# and create a Linux AppImage bundle file.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

# Halt and catch errors
set -eE
trap 'PREVIOUS_COMMAND=$THIS_COMMAND; THIS_COMMAND=$BASH_COMMAND' DEBUG
trap 'echo "FAILED COMMAND: $PREVIOUS_COMMAND"' ERR

if [ "root" != "$USER" ]; then
    echo "This script must be run as root..."
    exit
fi

if [[ "$(arch)" = "x86_64" ]] ; then

    LIB_PATH_ALT=lib64

else

    LIB_PATH_ALT=lib

fi

#################################################################################################
# Manage script traces to log file

mkdir -p ./logs
exec > >(tee ./logs/build-appimage.full.log) 2>&1

#################################################################################################

echo "04-build-appimage.sh : build digiKam AppImage bundle."
echo "-----------------------------------------------------"

#################################################################################################
# Pre-processing checks

. ./config.sh
. ./common.sh
ChecksRunAsRoot
StartScript
ChecksCPUCores
CentOS6Adjustments
. /opt/rh/devtoolset-6/enable

#################################################################################################

# Working directory
ORIG_WD="`pwd`"

DK_RELEASEID=`cat $ORIG_WD/data/RELEASEID.txt`

#################################################################################################

echo -e "---------- Build icons-set ressource\n"

cd $ORIG_WD/icon-rcc

rm -f CMakeCache.txt > /dev/null

cmake3 -DCMAKE_INSTALL_PREFIX="/usr" \
       -DCMAKE_BUILD_TYPE=debug \
       -DCMAKE_COLOR_MAKEFILE=ON \
       -Wno-dev \
       .

make -j$CPU_CORES

#################################################################################################

echo -e "---------- Prepare directories in bundle\n"

# Make sure we build from the /, parts of this script depends on that. We also need to run as root...
cd /

# Prepare the install location
rm -rf $APP_IMG_DIR/ || true
mkdir -p $APP_IMG_DIR/usr/bin
mkdir -p $APP_IMG_DIR/usr/etc
mkdir -p $APP_IMG_DIR/usr/share
mkdir -p $APP_IMG_DIR/usr/share/icons
mkdir -p $APP_IMG_DIR/usr/share/metainfo
mkdir -p $APP_IMG_DIR/usr/share/dbus-1/interfaces
mkdir -p $APP_IMG_DIR/usr/share/dbus-1/services

# make sure lib and lib64 are the same thing
mkdir -p $APP_IMG_DIR/usr/lib
mkdir -p $APP_IMG_DIR/usr/lib/libexec
mkdir -p $APP_IMG_DIR/usr/lib/libgphoto2
mkdir -p $APP_IMG_DIR/usr/lib/libgphoto2_port
cd $APP_IMG_DIR/usr
ln -s lib lib64

#################################################################################################

echo -e "---------- Copy Files in bundle\n"

cd $APP_IMG_DIR

# FIXME: How to find out which subset of plugins is really needed? I used strace when running the binary
cp -r /usr/plugins ./usr/
rm -fr ./usr/plugins/ktexteditor
rm -fr ./usr/plugins/kf5/parts
rm -fr ./usr/plugins/konsolepart.so

# copy runtime data files
cp -r /usr/share/digikam                  ./usr/share
cp -r /usr/share/showfoto                 ./usr/share
cp $ORIG_WD/icon-rcc/breeze.rcc           ./usr/share/digikam
cp $ORIG_WD/icon-rcc/breeze-dark.rcc      ./usr/share/digikam

cd $APP_IMG_DIR/usr/share/showfoto
ln -s ../digikam/breeze.rcc               breeze.rcc
ln -s ../digikam/breeze-dark.rcc          breeze-dark.rcc

cd $APP_IMG_DIR
cp $ORIG_WD/data/qt.conf                  ./usr/bin
cp -r /usr/share/lensfun                  ./usr/share
cp -r /usr/share/knotifications5          ./usr/share
cp -r /usr/share/kservices5               ./usr/share
cp -r /usr/share/kservicetypes5           ./usr/share
cp -r /usr/share/kxmlgui5                 ./usr/share
cp -r /usr/share/kf5                      ./usr/share
cp -r /usr/share/solid                    ./usr/share
cp -r /usr/share/OpenCV                   ./usr/share
cp -r /usr/share/dbus-1/interfaces/kf5*   ./usr/share/dbus-1/interfaces/
cp -r /usr/share/dbus-1/services/*kde*    ./usr/share/dbus-1/services/
cp -r /usr/$LIB_PATH_ALT/libexec/kf5      ./usr/lib/libexec/

# AppImage stream data file
cp -r /usr/share/metainfo/org.kde.digikam.appdata.xml   ./usr/share/metainfo/digikam.appdata.xml
cp -r /usr/share/metainfo/org.kde.showfoto.appdata.xml  ./usr/share/metainfo/showfoto.appdata.xml

# QWebEngine bin data files.
if [[ $DK_QTWEBENGINE = 1 ]] ; then
    cp -r /usr/resources ./usr
fi

# copy libgphoto2 drivers
find  /usr/lib/libgphoto2      -name "*.so" -type f -exec cp {} ./usr/lib/libgphoto2 \;      2>/dev/null
find  /usr/lib/libgphoto2_port -name "*.so" -type f -exec cp {} ./usr/lib/libgphoto2_port \; 2>/dev/null

# copy sane backends
cp -r /usr/lib/sane                       ./usr/lib
rm ./usr/lib/sane/*.la
cp -r /usr/etc/sane.d                     ./usr/etc

# copy i18n

# Qt translations files
if [[ -e /usr/translations ]]; then

    cp -r /usr/translations ./usr
    # optimizations
    rm -rf ./usr/translations/assistant*    || true
    rm -rf ./usr/translations/designer*     || true
    rm -rf ./usr/translations/linguist*     || true
    rm -rf ./usr/translations/qmlviewer*    || true
    rm -rf ./usr/translations/qtmultimedia* || true
    rm -rf ./usr/translations/qtscript*     || true
    rm -rf ./usr/translations/qtquick*      || true
    rm -rf ./usr/translations/qt_help*      || true
    rm -rf ./usr/translations/qtserialport* || true
    rm -rf ./usr/translations/qtwebsockets* || true

fi

# KF5 translations files
FILES=$(cat $ORIG_WD/logs/build-extralibs.full.log | grep /usr/share/locale | grep -e .qm -e .mo | cut -d' ' -f3)

for FILE in $FILES ; do
    cp --parents $FILE ./
done

# digiKam translations files
FILES=$(cat $ORIG_WD/logs/build-digikam.full.log | grep /usr/share/locale | grep -e .qm -e .mo | cut -d' ' -f3)

for FILE in $FILES ; do
    cp --parents $FILE ./
done

# digiKam icons files
FILES=$(cat $ORIG_WD/logs/build-digikam.full.log | grep /usr/share/icons/ | cut -d' ' -f3)

for FILE in $FILES ; do
    cp --parents $FILE ./
done

# Marble data and plugins files

cp -r /usr/$LIB_PATH_ALT/marble/plugins/ ./usr/bin/

cp -r /usr/share/marble/data             ./usr/bin/

# otherwise segfaults!?
cp $(ldconfig -p | grep /usr/$LIB_PATH_ALT/libsasl2.so.2    | cut -d ">" -f 2 | xargs) ./usr/lib/
cp $(ldconfig -p | grep /usr/$LIB_PATH_ALT/libGL.so.1       | cut -d ">" -f 2 | xargs) ./usr/lib/
cp $(ldconfig -p | grep /usr/$LIB_PATH_ALT/libGLU.so.1      | cut -d ">" -f 2 | xargs) ./usr/lib/

# Fedora 23 seemed to be missing SOMETHING from the Centos 6.7. The only message was:
# This application failed to start because it could not find or load the Qt platform plugin "xcb".
# Setting export QT_DEBUG_PLUGINS=1 revealed the cause.
# QLibraryPrivate::loadPlugin failed on "/usr/lib64/qt5/plugins/platforms/libqxcb.so" :
# "Cannot load library /usr/lib64/qt5/plugins/platforms/libqxcb.so: (/lib64/libEGL.so.1: undefined symbol: drmGetNodeTypeFromFd)"
# Which means that we have to copy libEGL.so.1 in too

# Otherwise F23 cannot load the Qt platform plugin "xcb"
cp $(ldconfig -p | grep /usr/$LIB_PATH_ALT/libEGL.so.1      | cut -d ">" -f 2 | xargs) ./usr/lib/

# let's not copy xcb itself, that breaks on dri3 systems https://bugs.kde.org/show_bug.cgi?id=360552
#cp $(ldconfig -p | grep libxcb.so.1 | cut -d ">" -f 2 | xargs) ./usr/lib/

# For Fedora 20
cp $(ldconfig -p | grep /usr/$LIB_PATH_ALT/libfreetype.so.6 | cut -d ">" -f 2 | xargs) ./usr/lib/

cp /usr/bin/digikam                 ./usr/bin
cp /usr/bin/showfoto                ./usr/bin
cp /usr/bin/kbuildsycoca5           ./usr/bin
cp /usr/bin/solid-hardware5         ./usr/bin

# QWebEngine runtime process
[[ -e /usr/libexec/QtWebEngineProcess ]] && cp /usr/libexec/QtWebEngineProcess ./usr/bin

# For Solid action when camera is connected to computer
cp /usr/bin/qdbus                   ./usr/share/digikam/utils
sed -i "/Exec=/c\Exec=digikam-camera downloadFromUdi %i" ./usr/share/solid/actions/digikam-opencamera.desktop

#################################################################################################

echo -e "---------- Scan dependencies recurssively\n"

CopyReccursiveDependencies /usr/bin/digikam                  ./usr/lib
CopyReccursiveDependencies /usr/bin/showfoto                 ./usr/lib
CopyReccursiveDependencies /usr/plugins/platforms/libqxcb.so ./usr/lib

FILES=$(ls /usr/$LIB_PATH_ALT/libdigikam*.so)

for FILE in $FILES ; do
    CopyReccursiveDependencies ${FILE} ./usr/lib
done

FILES=$(ls /usr/plugins/imageformats/*.so)

for FILE in $FILES ; do
    CopyReccursiveDependencies ${FILE} ./usr/lib
done

# Copy in the indirect dependencies
FILES=$(find . -type f -executable)

for FILE in $FILES ; do
    CopyReccursiveDependencies ${FILE} ./usr/lib
done

#################################################################################################

echo -e "---------- Clean-up Bundle Directory Contents\n"

# The following are assumed to be part of the base system

# This list is taken from linuxdeplotqt
# [https://github.com/probonopd/linuxdeployqt/blob/master/tools/linuxdeployqt/excludelist.h]
# NOTE: libglapi is included explicity in Krita exclude list.
EXCLUDE_FILES="\
ld-linux.so.2 \
ld-linux-x86-64.so.2 \
libanl.so.1 \
libasound.so.2 \
libBrokenLocale.so.1 \
libcidn.so.1 \
libcom_err.so.2 \
libcrypt.so.1 \
libc.so.6 \
libdl.so.2 \
libdrm.so.2 \
libexpat.so.1 \
libfontconfig.so.1 \
libfreetype.so.6 \
libgcc_s.so.1 \
libgdk_pixbuf-2.0.so.0 \
libgio-2.0.so.0 \
libglapi.so.0 \
libglib-2.0.so.0 \
libGL.so.1 \
libgobject-2.0.so.0 \
libgpg-error.so.0 \
libharfbuzz.so.0 \
libICE.so.6 \
libjack.so.0 \
libkeyutils.so.1 \
libm.so.6 \
libmvec.so.1 \
libnsl.so.1 \
libnss_compat.so.2 \
libnss_db.so.2 \
libnss_dns.so.2 \
libnss_files.so.2 \
libnss_hesiod.so.2 \
libnss_nisplus.so.2 \
libnss_nis.so.2 \
libp11-kit.so.0 \
libpango-1.0.so.0 \
libpangocairo-1.0.so.0 \
libpangoft2-1.0.so.0 \
libpthread.so.0 \
libresolv.so.2 \
librt.so.1 \
libSM.so.6 \
libstdc++.so.6 \
libthai.so.0 \
libthread_db.so.1 \
libusb-1.0.so.0 \
libutil.so.1 \
libuuid.so.1 \
libX11.so.6 \
libxcb.so.1 \
libz.so.1 \
"

for FILE in $EXCLUDE_FILES ; do
    if [[ -f usr/lib/${FILE} ]] ; then
        echo -e "   ==> ${FILE} will be removed for the bundle"
        rm -f usr/lib/${FILE}
    fi
done

# This list is taken from older AppImage build script from krita
# NOTE: libopenal => see bug 390162.
#       libdbus-1 => see Krita rules.
EXTRA_EXCLUDE_FILES="\
libgssapi_krb5.so.2 \
libgssapi.so.3 \
libhcrypto.so.4 \
libheimbase.so.1 \
libheimntlm.so.0 \
libhx509.so.5 \
libidn.so.11 \
libk5crypto.so.3 \
libkrb5.so.26 \
libkrb5.so.3 \
libkrb5support.so.0 \
libpcre.so.3 \
libroken.so.18 \
libsasl2.so.2 \
libwind.so.0 \
libopenal.so.1 \
libdbus-1.so.3 \
"

#liblber-2.4.so.2       # needed for debian wheezy
#libldap_r-2.4.so.2     # needed for debian wheezy

#libffi.so.6            # needed for Ubuntu 11.04
#libxcb-glx.so.0        # needed for Ubuntu 11.04

for FILE in $EXCLUDE_FILES ; do
    if [[ -f usr/lib/${FILE} ]] ; then
        echo -e "   ==> ${FILE} will be removed for the bundle"
        rm -f usr/lib/${FILE}
    fi
done

ln -s libssl.so.10 usr/lib/libssl.so || true

# We don't bundle the developer stuff
rm -rf usr/include         || true
rm -rf usr/lib/cmake3      || true
rm -rf usr/lib/pkgconfig   || true
rm -rf usr/share/ECM/      || true
rm -rf usr/share/gettext   || true
rm -rf usr/share/pkgconfig || true

#################################################################################################
# See LFS instruction: http://www.linuxfromscratch.org/lfs/view/systemd/chapter05/stripping.html

echo -e "---------- Strip Binaries Files \n"

if [[ $DK_DEBUG = 1 ]] ; then
    FILES=$(find . -type f -executable | grep -Ev '(digikam|showfoto|exiv2)')
else
    FILES=$(find . -type f -executable)
fi

for FILE in $FILES ; do
    echo -en "Strip symbols in: $FILE"
    /usr/bin/strip --strip-debug ${FILE}
    echo -e " ==> OK"
done

#################################################################################################

echo -e "---------- Strip Configuration Files \n"

# Since we set $APP_IMG_DIR as the prefix, we need to patch it away too (FIXME)
# Probably it would be better to use /app as a prefix because it has the same length for all apps
cd usr/ ; find . -type f -exec sed -i -e 's|$APP_IMG_DIR/usr/|./././././././././|g' {} \; ; cd  ..

# On openSUSE Qt is picking up the wrong libqxcb.so
# (the one from the system when in fact it should use the bundled one) - is this a Qt bug?
# Also, Krita has a hardcoded /usr which we patch away
cd usr/ ; find . -type f -exec sed -i -e 's|/usr|././|g' {} \; ; cd ..

# We do not bundle this, so let's not search that inside the AppImage.
# Fixes "Qt: Failed to create XKB context!" and lets us enter text
sed -i -e 's|././/share/X11/|/usr/share/X11/|g' ./usr/plugins/platforminputcontexts/libcomposeplatforminputcontextplugin.so
sed -i -e 's|././/share/X11/|/usr/share/X11/|g' ./usr/lib/libQt5XcbQpa.so.5

#################################################################################################

cd /

APP=digikam

#if [[ $DK_DEBUG = 1 ]] ; then
#    DEBUG_SUF="-debug"
#fi

if [[ $DK_QTWEBENGINE = 1 ]] ; then
    WEB_BROWSER="-qtwebengine"
else
    WEB_BROWSER="-qtwebkit"
fi

if [[ "$ARCH" = "x86_64" ]] ; then
    APPIMAGE=$APP"-"$DK_RELEASEID$DK_EPOCH$WEB_BROWSER"-x86-64$DEBUG_SUF.appimage"
elif [[ "$ARCH" = "i686" ]] ; then
    APPIMAGE=$APP"-"$DK_RELEASEID$DK_EPOCH$WEB_BROWSER"-i386$DEBUG_SUF.appimage"
fi

echo -e "---------- Create Bundle with AppImage SDK stage1\n"

# Source functions

if [[ ! -s ./functions.sh ]] ; then
    wget -q https://github.com/probonopd/AppImages/raw/master/functions.sh -O ./functions.sh
fi

# Install desktopintegration in usr/bin/digikam.wrapper
cd $APP_IMG_DIR

# We will use a dedicated bash script to run inside the AppImage to be sure that XDG_* variable are set for Qt5
cp ${ORIG_WD}/data/AppRun ./

# desktop integration files

cp /usr/share/applications/org.kde.digikam.desktop ./digikam.desktop
cp /usr/share/icons/hicolor/64x64/apps/digikam.png ./digikam.png

mkdir -p $APP_IMG_DIR/usr/share/icons/default/128x128/apps
cp -r /usr/share/icons/hicolor/128x128/apps/digikam.png ./usr/share/icons/default/128x128/apps/digikam.png

mkdir -p $APP_IMG_DIR/usr/share/icons/default/128x128/mimetypes
cp -r /usr/share/icons/hicolor/128x128/apps/digikam.png ./usr/share/icons/default/128x128/mimetypes/application-vnd.digikam.png

mkdir -p $ORIG_WD/bundle
rm -f $ORIG_WD/bundle/* || true

echo -e "---------- Create Bundle with AppImage SDK stage2\n"

cd /

# Get right version of Appimage toolkit.

if [[ "$ARCH" = "x86_64" ]] ; then
    APPIMGBIN=AppImageTool-x86_64.AppImage
elif [[ "$ARCH" = "i686" ]] ; then
    APPIMGBIN=AppImageTool-i686.AppImage
fi

if [[ ! -s ./$APPIMGBIN ]] ; then
    wget -q https://github.com/AppImage/AppImageKit/releases/download/continuous/$APPIMGBIN -O ./$APPIMGBIN
fi

chmod a+x ./$APPIMGBIN

./$APPIMGBIN $APP_IMG_DIR/ $ORIG_WD/bundle/$APPIMAGE
chmod a+rwx $ORIG_WD/bundle/$APPIMAGE

#################################################################################################
# Show resume information and future instructions to host installer file to remote server

echo -e "\n---------- Compute package checksums for digiKam $DK_RELEASEID\n"  > $ORIG_WD/bundle/$APPIMAGE.sum
echo    "File       : $APPIMAGE"                                             >> $ORIG_WD/bundle/$APPIMAGE.sum
echo -n "Size       : "                                                      >> $ORIG_WD/bundle/$APPIMAGE.sum
du -h "$ORIG_WD/bundle/$APPIMAGE"     | { read first rest ; echo $first ; }  >> $ORIG_WD/bundle/$APPIMAGE.sum
echo -n "MD5 sum    : "                                                      >> $ORIG_WD/bundle/$APPIMAGE.sum
md5sum "$ORIG_WD/bundle/$APPIMAGE"    | { read first rest ; echo $first ; }  >> $ORIG_WD/bundle/$APPIMAGE.sum
echo -n "SHA1 sum   : "                                                      >> $ORIG_WD/bundle/$APPIMAGE.sum
sha1sum "$ORIG_WD/bundle/$APPIMAGE"   | { read first rest ; echo $first ; }  >> $ORIG_WD/bundle/$APPIMAGE.sum
echo -n "SHA256 sum : "                                                      >> $ORIG_WD/bundle/$APPIMAGE.sum
sha256sum "$ORIG_WD/bundle/$APPIMAGE" | { read first rest ; echo $first ; }  >> $ORIG_WD/bundle/$APPIMAGE.sum

# Checksums to post on Phabricator at release time.
sha256sum "$ORIG_WD/bundle/$APPIMAGE" > $ORIG_WD/bundle/sha256_release.sum

if [[ $DK_SIGN = 1 ]] ; then

    cat ~/.gnupg/dkorg-gpg-pwd.txt | gpg --batch --yes --passphrase-fd 0 -sabv "$ORIG_WD/bundle/$APPIMAGE"
    mv -f $ORIG_WD/bundle/$APPIMAGE.asc $ORIG_WD/bundle/$APPIMAGE.sig

    echo    "File       : $APPIMAGE.sig"                                             >> $ORIG_WD/bundle/$APPIMAGE.sum
    echo -n "Size       : "                                                          >> $ORIG_WD/bundle/$APPIMAGE.sum
    du -h "$ORIG_WD/bundle/$APPIMAGE.sig"     | { read first rest ; echo $first ; }  >> $ORIG_WD/bundle/$APPIMAGE.sum
    echo -n "MD5 sum    : "                                                          >> $ORIG_WD/bundle/$APPIMAGE.sum
    md5sum "$ORIG_WD/bundle/$APPIMAGE.sig"    | { read first rest ; echo $first ; }  >> $ORIG_WD/bundle/$APPIMAGE.sum
    echo -n "SHA1 sum   : "                                                          >> $ORIG_WD/bundle/$APPIMAGE.sum
    sha1sum "$ORIG_WD/bundle/$APPIMAGE.sig"   | { read first rest ; echo $first ; }  >> $ORIG_WD/bundle/$APPIMAGE.sum
    echo -n "SHA256 sum : "                                                          >> $ORIG_WD/bundle/$APPIMAGE.sum
    sha256sum "$ORIG_WD/bundle/$APPIMAGE.sig" | { read first rest ; echo $first ; }  >> $ORIG_WD/bundle/$APPIMAGE.sum

    # Checksums to post on Phabricator at release time.
    sha256sum "$ORIG_WD/bundle/$APPIMAGE.sig" >> $ORIG_WD/bundle/sha256_release.sum

fi

cat $ORIG_WD/bundle/$APPIMAGE.sum

if [[ $DK_UPLOAD = 1 ]] ; then

    echo -e "---------- Cleanup older bundle AppImage files from files.kde.org repository \n"

    if [[ "$ARCH" = "x86_64" ]] ; then
        ssh $DK_UPLOADURL rm -f $DK_UPLOADDIR*-x86-64*.appimage*
    elif [[ "$ARCH" = "i686" ]] ; then
        ssh $DK_UPLOADURL rm -f $DK_UPLOADDIR*-i386*.appimage*
    fi

    echo -e "---------- Upload new bundle AppImage files to files.kde.org repository \n"

    rsync -r -v --progress -e ssh $ORIG_WD/bundle/$APPIMAGE     $DK_UPLOADURL:$DK_UPLOADDIR
    scp $ORIG_WD/bundle/$APPIMAGE.sum $DK_UPLOADURL:$DK_UPLOADDIR

    if [[ $DK_SIGN = 1 ]] ; then
        scp $ORIG_WD/bundle/$APPIMAGE.sig $DK_UPLOADURL:$DK_UPLOADDIR
    fi

else
    echo -e "\n------------------------------------------------------------------"
    curl https://download.kde.org/README_UPLOAD
    echo -e "------------------------------------------------------------------\n"
fi

#################################################################################################

TerminateScript
