#
# Copyright (c) 2015-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

add_subdirectory(blurfx)
add_subdirectory(charcoal)
add_subdirectory(colorfx)
add_subdirectory(distortionfx)
add_subdirectory(emboss)
add_subdirectory(filmgrain)
add_subdirectory(oilpaint)
add_subdirectory(raindrop)
