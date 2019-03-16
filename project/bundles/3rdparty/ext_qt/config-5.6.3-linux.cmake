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
            -verbose                          # Print details while configuration
            -opensource                       # Build open-source framework edition
            -confirm-license                  # Silency ack the license
            -opengl desktop                   # Enable OpenGL support from Desktop
            -openssl                          # Use SSL from system
            -qt-sql-sqlite                    # Use SQlite from the system

            # Compilation rules to disable.
            -nomake tests                     # Do not build test codes
            -nomake examples                  # Do not build basis example codes
            -no-compile-examples              # Do not build extra example codes
            -no-icu                           # Do not support ICU: https://wiki.qt.io/Qt_5_ICU
            -no-qml-debug                     # Do not support debug with QML
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
            -qt-libpng                        # Use internal PNG lib
            -qt-libjpeg                       # Use internal JPEG lib
            -qt-pcre                          # Use internal regular expression lib https://doc.qt.io/archives/qt-5.8/qtcore-attribution-pcre.html
            -qt-harfbuzz                      # Use internal OpenType lib
            -qt-freetype                      # Use internal font rendering lib https://doc.qt.io/qt-5/qtgui-attribution-freetype.html
            -qt-xcb                           # Use internal X11 lib http://doc.qt.io/qt-5/linux-requirements.html
            -qt-xkbcommon-x11                 # Use internal X11 keyboard lib https://doc.qt.io/qt-5/qtgui-attribution-xkbcommon.html

            # Qt5 Framework components to disable.
            -skip qtactiveqt                  # No need ActiveX support
            -skip qtandroidextras             # For embeded devices only
            -skip qtmacextras                 # For MacOS devices only
            -skip qtconnectivity              # For embeded devices only
            -skip qtscript                    # No need scripting (deprecated)
            -skip qtsensors                   # For embeded devices only
            -skip qtwayland                   # Specific to Linux
            -skip qtdoc                       # No need documentation
            -skip qtenginio                   # No need backend-as-service support
            -skip qtlocation                  # No need geolocation
            -skip qt3d                        # 3D core
            -skip qtcanvas3d                  # 3D extensions
            -skip qtgraphicaleffects          # Advanced graphical effects in GUI
            -skip qtquickcontrols2            # QtQuick support for QML
            -skip qtwebview                   # QML extension for QWebEngine

#           -skip qtwebchannel                # Do not disable QtWebChannel support ==> need for QWebEngine !
#           -skip qtquickcontrols             # Do not disable QtQuick support ==> need for QWebEngine !
#           -skip qtdeclarative               # Do not disable QML support ==> need for QWebEngine !
)

MESSAGE(STATUS "Use Linux ${QT_VERSION} configuration:")
MESSAGE(STATUS ${QT_CONFIG})
