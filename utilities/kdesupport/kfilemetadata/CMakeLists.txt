#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(baloowrap_SRCS baloowrap.cpp)

include_directories($<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::FileMetaData,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(baloowrap_src OBJECT ${baloowrap_SRCS})
add_library(baloowrap STATIC $<TARGET_OBJECTS:baloowrap_src>)

target_link_libraries(baloowrap PRIVATE KF5::FileMetaData KF5::I18n)
