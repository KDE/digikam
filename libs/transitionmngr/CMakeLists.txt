#
# Copyright (c) 2017 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(libtransitionmngr_SRCS
    effectpreview.cpp
    effectmngr.cpp
    effectmngr_p.cpp
    effectmngr_p_pan.cpp
    effectmngr_p_zoom.cpp
    frameutils.cpp
    transitionpreview.cpp
    transitionmngr.cpp
    transitionmngr_p.cpp
    transitionmngr_p_abstract.cpp
    transitionmngr_p_blur.cpp
    transitionmngr_p_lines.cpp
    transitionmngr_p_push.cpp
    transitionmngr_p_slide.cpp
    transitionmngr_p_squares.cpp
    transitionmngr_p_swap.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(digikamlibtransitionmngr_src OBJECT ${libtransitionmngr_SRCS})
