#
# Copyright (c) 2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(ksaneiface_SRCS ksaneaction.cpp scandialog.cpp saveimgthread.cpp)

include_directories(
    $<TARGET_PROPERTY:${LIBKSANE_LIBRARIES},INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::KExiv2,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::ConfigGui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(ksaneiface_src OBJECT ${ksaneiface_SRCS})
add_library(ksaneiface STATIC $<TARGET_OBJECTS:ksaneiface_src>)

target_link_libraries(ksaneiface PRIVATE ${LIBKSANE_LIBRARIES} KF5::I18n)

