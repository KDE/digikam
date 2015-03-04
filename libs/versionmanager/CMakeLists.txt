#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libversionmanager_SRCS
    versionmanager.cpp
)

add_library(versionmanager STATIC ${libversionmanager_SRCS})

target_link_libraries(versionmanager

                      dimg
)