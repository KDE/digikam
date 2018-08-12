#
# Copyright (c) 2010-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# =======================================================
# Stand alone version of ExpoBlending tool

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

add_subdirectory(fbdemo)
#add_subdirectory(twdemo)
add_subdirectory(sialis)

if(ENABLE_QWEBENGINE)
    add_subdirectory(fbinter)
endif()
