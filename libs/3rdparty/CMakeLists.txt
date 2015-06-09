#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(libkde3rdparty_SRCS
    kmemoryinfo/kmemoryinfo.cpp
    kinotify/kinotify.cpp
)

#add_library( OBJECT )
#add_library( STATIC $<TARGET_OBJECTS:>)

add_library(3rdparty_src OBJECT ${libkde3rdparty_SRCS})
#add_library(3rdparty STATIC $<TARGET_OBJECTS:3rdparty_src>)

#target_link_libraries(3rdparty
#                      PRIVATE
#                      Qt5::Core
#)
