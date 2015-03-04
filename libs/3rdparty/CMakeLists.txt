#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libkde3rdparty_SRCS
    kmemoryinfo/kmemoryinfo.cpp
    kinotify/kinotify.cpp
)

add_library(3rdparty STATIC ${libkde3rdparty_SRCS})

target_link_libraries(3rdparty

                      Qt5::Core
)