#
# Copyright (c) 2010-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# =======================================================

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories(
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
)

set(loadandrun_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/loadandrun.cpp)

add_executable(loadandrun ${loadandrun_SRCS})

target_link_libraries(loadandrun
                      digikamcore

                      Qt5::Gui
                      Qt5::Core
)
