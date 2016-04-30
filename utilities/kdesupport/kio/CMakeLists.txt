#
# Copyright (c) 2010-2016 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Mohamed Anwer, <m dot anwer at gmx dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(libkiowrapper_SRCS
    kiowrapper.cpp
)

include_directories(
    $<TARGET_PROPERTY:KF5::KIOCore,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::KIOWidgets,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(kiowrapper_src OBJECT ${libkiowrapper_SRCS})
