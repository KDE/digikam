#
# Copyright (c) 2015-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

add_subdirectory(converttodng)
add_subdirectory(converttopng)
add_subdirectory(converttopgf)
add_subdirectory(converttojpeg)
add_subdirectory(converttotiff)

if(Jasper_FOUND)
    add_subdirectory(converttojp2)
endif()
