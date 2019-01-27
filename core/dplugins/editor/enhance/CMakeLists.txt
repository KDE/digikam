#
# Copyright (c) 2015-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

add_subdirectory(antivignetting)
add_subdirectory(blur)
add_subdirectory(hotpixels)
add_subdirectory(redeye)
add_subdirectory(sharpen)
add_subdirectory(restoration)
add_subdirectory(localcontrast)
add_subdirectory(noisereduction)
add_subdirectory(lensdistortion)

if(LensFun_FOUND)
    add_subdirectory(lensautofix)
endif()

# Not yet finalized
#add_subdirectory(healingclone)
