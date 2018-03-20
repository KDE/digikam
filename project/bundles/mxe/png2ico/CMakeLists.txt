#
# Copyright (c) 2010-2017 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

cmake_minimum_required(VERSION 3.0.0)

FIND_PACKAGE(Qt5Core REQUIRED)
FIND_PACKAGE(Qt5Gui  REQUIRED)

include_directories(${Qt5Core_INCLUDE_DIRS}
                    ${Qt5Gui_INCLUDE_DIRS}
                    ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(png2ico
               png2ico.cpp
               qicohandler.cpp
               qcurhandler.cpp
               qanihandler.cpp
)

target_link_libraries(png2ico Qt5::Core Qt5::Gui)

install(TARGETS png2ico DESTINATION bin)
