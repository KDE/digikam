# Script to build Qt for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

LIST(APPEND QT_CONFIG
            # Framework install path.
            -prefix ${EXTPREFIX_qt}

            # Compilation rules to enable.
            -release                          # No debug symbols
            -verbose                          # Print details while configuration
            -opensource                       # Build open-source framework edition
            -confirm-license                  # Silency ack the license
            -opengl desktop                   # Enable OpenGL support from Desktop
            -fontconfig                       # Enable Fontconfig support
            -openssl-linked                   # Use SSL from system and link
            -sql-sqlite                       # Enable Sqlite plugin support
            -I ${EXTPREFIX_qt}/include
            -L ${EXTPREFIX_qt}/lib
            -L ${EXTPREFIX_qt}/lib64

            # Compilation rules to disable.
            -nomake tests                     # Do not build test codes
            -nomake examples                  # Do not build basis example codes
            -no-compile-examples              # Do not build extra example codes
            -no-icu                           # Do not support ICU: https://wiki.qt.io/Qt_5_ICU
            -no-mtdev                         # Do not support multi-touch
            -no-libproxy                      # Do not support network proxy
            -no-pch                           # Do not support pre-compiled header
            -no-journald                      # Do not support journald log
            -no-syslog                        # Do not support syslog log
            -no-tslib                         # Do not support touch screen
            -no-directfb                      # Do not support Direct Framebuffer
            -no-linuxfb                       # Do not support Linux Framebuffer

            # Specific 3rdParty libraries to enable.
            -qt-zlib                          # Use internal Z compression lib
            -qt-pcre                          # Use internal regular expression lib https://doc.qt.io/archives/qt-5.8/qtcore-attribution-pcre.html
            -qt-harfbuzz                      # Use internal OpenType lib
            -system-freetype                  # Use system font rendering lib https://doc.qt.io/qt-5/qtgui-attribution-freetype.html
            -qt-xcb                           # Use internal X11 lib http://doc.qt.io/qt-5/linux-requirements.html
            -qt-xkbcommon-x11                 # Use internal X11 keyboard lib https://doc.qt.io/qt-5/qtgui-attribution-xkbcommon.html

            # Qt5 Framework components to disable.
            -skip qt3d                        # 3D core
            -skip qtactiveqt                  # No need ActiveX support
            -skip qtandroidextras             # For embeded devices only
            -skip qtwinextras                 # For Windows devices only
            -skip qtmacextras                 # For MacOS devices only
            -skip qtcanvas3d                  # 3D extensions
            -skip qtcharts                    # No need data models charts support
            -skip qtconnectivity              # For embeded devices only
            -skip qtscript                    # No need scripting (deprecated)
            -skip qtdatavis3d                 # no need 3D data visualizations support
            -skip qtdoc                       # No need documentation
            -skip qtenginio                   # No need backend-as-service support
            -skip qtgamepad                   # No need gamepad hardware support.
            -skip qtgraphicaleffects          # Advanced graphical effects in GUI
            -skip qtlocation                  # No need geolocation
            -skip qtquickcontrols2            # QtQuick support for QML
            -skip qtmultimedia                # No need multimedia support (replaced by QtAV+ffmpeg)
            -skip qtnetworkauth               # No need network authentification support.
            -skip qtpurchasing                # No need in-app purchase of products support
            -skip qtremoteobjects             # No need sharing QObject properties between processes support
            -skip qtserialport                # No need serial port support
            -skip qtscxml                     # No need SCXML state machines support
            -skip qtsensors                   # For embeded devices only
            -skip qtspeech                    # No need speech synthesis support
            -skip qttranslations              # No need translation tools.
            -skip qtvirtualkeyboard           # No need virtual keyboard support
            -skip qtwayland                   # Specific to Linux
            -skip qtwebsockets                # No need websocket support
            -skip qtwebchannel                # No need sharing QObject properties with JS
            -skip qtwebview                   # QML extension for QWebEngine
            -skip qtwebglplugin               # No need browser OpenGL extention support

            -skip qtwebengine                 # No need Chromium browser support (QtWebkit instead)
            -skip qtwebchannel                # QtWebChannel support ==> QWebEngine dependency
            -skip qtquickcontrols             # QtQuick support ==> QWebEngine dependency
)

MESSAGE(STATUS "Use Linux ${QT_VERSION} configuration:")
MESSAGE(STATUS ${QT_CONFIG})
