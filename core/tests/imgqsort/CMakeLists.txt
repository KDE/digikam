#
# Copyright (c) 2010-2018, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

include_directories(
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Test,INTERFACE_INCLUDE_DIRECTORIES>
)

# --------------------------------------------------------------

macro(IMGQSORT_BUILD_STANDALONE)

    set(_filename ${ARGV0})
    string(REPLACE "." ";" _temp ${_filename})
    list(GET _temp 0 _target)

    set(${_target}_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/${_filename})

    add_executable(${_target} ${${_target}_SRCS})

    target_link_libraries(${_target}
                          digikamcore
                          Qt5::Core

                          ${OpenCV_LIBRARIES}
    )

endmacro()

IMGQSORT_BUILD_STANDALONE(detectblur.cpp)
IMGQSORT_BUILD_STANDALONE(detectcompression.cpp)
IMGQSORT_BUILD_STANDALONE(detectnoise.cpp)
IMGQSORT_BUILD_STANDALONE(detectoverexpo.cpp)

# --------------------------------------------------------------

macro(IMGQSORT_BUILD_UNITTEST)

    set(_filename ${ARGV0})
    string(REPLACE "." ";" _temp ${_filename})
    list(GET _temp 0 _target)

    set(${_target}_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/${_filename})

    add_executable(${_target} ${${_target}_SRCS})

    target_link_libraries(${_target}
                          digikamcore
                          Qt5::Core
                          Qt5::Test

                          ${OpenCV_LIBRARIES}
    )

    ecm_mark_as_test(${_target})

endmacro()

IMGQSORT_BUILD_UNITTEST(imgqsorttest.cpp)

