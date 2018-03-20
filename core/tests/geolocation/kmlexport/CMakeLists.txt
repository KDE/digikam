#
# Copyright (c) 2010-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

add_executable(test_geoparsing
               test_geoparsing.cpp
)

target_link_libraries(test_geoparsing
                      Qt5::Test
)

add_test(test_geoparsing ${EXECUTABLE_OUTPUT_PATH}/test_geoparsing)
