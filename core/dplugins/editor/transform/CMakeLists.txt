#
# Copyright (c) 2015-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

add_subdirectory(autocrop)
add_subdirectory(freerotation)
add_subdirectory(resize)
add_subdirectory(perspective)
add_subdirectory(ratiocrop)
add_subdirectory(shear)

if(Lqr-1_FOUND)
    add_subdirectory(contentawareresize)
endif()
