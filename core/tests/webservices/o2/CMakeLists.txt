#
# Copyright (c) 2010-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

DISABLE_CLANG_COMPILER_WARNINGS("4.99.99" "-Wno-extra-semi")

add_subdirectory(fbdemo)
add_subdirectory(twdemo)

if (Qt5Qml_FOUND AND Qt5WebView_FOUND)
    add_subdirectory(sialis)
endif()

if(ENABLE_QWEBENGINE)
    add_subdirectory(fbinter)
endif()
