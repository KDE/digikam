#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(akonadiiface_SRCS akonadiiface.cpp)

add_library(akonadiiface_src OBJECT ${akonadiiface_SRCS})
add_library(akonadiiface STATIC $<TARGET_OBJECTS:akonadiiface_src>)

target_link_libraries(akonadiiface PRIVATE Qt5::Core KF5::I18n)

if(KF5AkonadiContact_FOUND)
    target_link_libraries(akonadiiface
#                          KF5::AkonadiCore
                          KF5::AkonadiContact
                          KF5::KIOCore
                         )
endif()

