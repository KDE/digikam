#
# Copyright (c) 2010-2017, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

# LibRaw use C++ exceptions
kde_enable_exceptions()

add_subdirectory(libraw)

set(librawengine_SRCS
    drawdecoder.cpp
    drawdecoder_p.cpp
    drawdecodersettings.cpp
    rawinfo.cpp
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/libraw/libraw
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(rawengine_srcs OBJECT ${librawengine_SRCS})
