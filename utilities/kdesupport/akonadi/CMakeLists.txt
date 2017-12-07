#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(akonadiiface_SRCS akonadiiface.cpp)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

if(KF5AkonadiContact_FOUND)
    include_directories(
        $<TARGET_PROPERTY:KF5::AkonadiCore,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:KF5::Contacts,INTERFACE_INCLUDE_DIRECTORIES>

        $<TARGET_PROPERTY:KF5::CoreAddons,INTERFACE_INCLUDE_DIRECTORIES>
    )
endif()

add_library(akonadiiface_src OBJECT ${akonadiiface_SRCS})
add_library(akonadiiface STATIC $<TARGET_OBJECTS:akonadiiface_src>)

target_link_libraries(akonadiiface PRIVATE Qt5::Core KF5::I18n)

if(KF5AkonadiContact_FOUND)
    target_include_directories(akonadiiface_src PRIVATE $<TARGET_PROPERTY:KF5::AkonadiContact,INTERFACE_INCLUDE_DIRECTORIES>)
    target_link_libraries(akonadiiface PRIVATE KF5::AkonadiContact)
endif()
