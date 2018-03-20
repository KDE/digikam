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
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
)

set(raw2png_SRCS raw2png.cpp)
add_executable(raw2png ${raw2png_SRCS})
target_link_libraries(raw2png
                      digikamcore

                      Qt5::Gui
                      Qt5::Core
)

set(libinfo_SRCS libinfo.cpp)
add_executable(libinfo ${libinfo_SRCS})
target_link_libraries(libinfo
                      digikamcore

                      Qt5::Gui
                      Qt5::Core
)

# -- LibRaw CLI Samples Compilation --------------------------------------------------------------------------------

# A small macro so that this is a bit cleaner

macro(LIBRAW_BUILD_SAMPLES)

    set(_filename ${ARGV0})
    string(REPLACE "." ";" _temp ${_filename})
    list(GET _temp 0 _target) 

    set(${_target}_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/rawengine/libraw/samples/${_filename})
    set_source_files_properties(${${_target}_SRCS} PROPERTIES COMPILE_FLAGS -w)

    add_executable(${_target} ${${_target}_SRCS})

    target_link_libraries(${_target}
                          ${OPENMP_LDFLAGS}
                          ${CMAKE_THREAD_LIBS_INIT}
                          digikamcore
                          libraw
                          libdng
    )

    if(WIN32)
        target_link_libraries(${_target} ws2_32)
    endif()

endmacro()

LIBRAW_BUILD_SAMPLES(simple_dcraw.cpp)
LIBRAW_BUILD_SAMPLES(mem_image.cpp)
LIBRAW_BUILD_SAMPLES(dcraw_emu.cpp)
LIBRAW_BUILD_SAMPLES(4channels.cpp)
LIBRAW_BUILD_SAMPLES(unprocessed_raw.cpp)
LIBRAW_BUILD_SAMPLES(raw-identify.cpp)
LIBRAW_BUILD_SAMPLES(multirender_test.cpp)
LIBRAW_BUILD_SAMPLES(postprocessing_benchmark.cpp)

# C sample files which must be compiled only if LCMS v2 is detected, to prevent broken compilation

if(LCMS2_FOUND)
   if(MSVC)
        LIBRAW_BUILD_SAMPLES(half_mt_win32.c)
   else()
        LIBRAW_BUILD_SAMPLES(dcraw_half.c)
        LIBRAW_BUILD_SAMPLES(half_mt.c)
   endif()
endif()
