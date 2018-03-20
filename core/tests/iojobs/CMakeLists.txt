#
# Copyright (c) 2015, Mohamed Anwer, <m dot anwer at gmx dot com>
# Copyright (c) 2010-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories(
    $<TARGET_PROPERTY:Qt5::Test,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
)

set(IOJobsTest_SRCS iojobstest.cpp)

add_executable(IOJobsTest ${IOJobsTest_SRCS})

ecm_mark_nongui_executable(IOJobsTest)

target_link_libraries(IOJobsTest
                      digikamgui

                      Qt5::Core
                      Qt5::Test
                     )
