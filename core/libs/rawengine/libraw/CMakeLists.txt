#
# Copyright (c) 2010-2017, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# -- General definitions rules ------------------------------------------------------------------------

# To prevent warnings from M$ compiler

if(WIN32 AND MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_ATL_SECURE_NO_WARNINGS)
    add_definitions(-D_AFX_SECURE_NO_WARNINGS)
endif()

# Under Windows, use specific flag to compile.

if(WIN32)
    add_definitions(-DDJGPP)
endif()

# Adjust compiler options to compile fine.

if(NOT MSVC)

    while(CMAKE_CXX_FLAGS MATCHES "-Werror=return-type")
        string(REPLACE "-Werror=return-type" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endwhile()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++11-narrowing" )

endif()

# We always use LCMS version 2

add_definitions(-DUSE_LCMS2)

# -- Check dependencies --------------------------------------------------------------------------------

message(STATUS "")
message(STATUS "--------------------------------------------------")
message(STATUS "digiKam RawEngine dependencies checks:")
message(STATUS "")

include(MacroOpenMP)
DETECT_OPENMP()

if(OPENMP_FOUND)
    message(STATUS "RawEngine will be compiled with OpenMP support")
else()
    message(STATUS "RawEngine will not be compiled with OpenMP support")
endif()

# Flag to use libjasper with LibRaw RedCine codec

if(Jasper_FOUND)
    add_definitions(-DUSE_JASPER)
    include_directories(${JASPER_INCLUDE_DIR})
    message(STATUS "RawEngine will be compiled with RedCine codec")
else()
    message(STATUS "RawEngine will not be compiled with RedCine codec")
endif()

# JPEG library check

message(STATUS "Looking for LibJpeg")
if(JPEG_FOUND)
    # JPEG lib version is done on top level through MacroJPEG.cmake
    if (${JPEG_LIB_VERSION} LESS 80)
        set(JPEG8_FOUND FALSE)
    else()
        set(JPEG8_FOUND TRUE)
    endif()
endif()

# Flag to use libjpeg with LibRaw DNG lossy codec

if(JPEG8_FOUND)
    add_definitions(-DUSE_JPEG)
    add_definitions(-DUSE_JPEG8)
    include_directories(${JPEG_INCLUDE_DIR})
    message(STATUS "RawEngine will be compiled with DNG lossy codec")
else()
    message(STATUS "RawEngine will not be compiled with DNG lossy codec")
endif()

message(STATUS "Looking for PThreads")
set(PTHREADS_FOUND (CMAKE_USE_PTHREADS_INIT OR CMAKE_USE_WIN32_THREADS_INIT))

message(STATUS "--------------------------------------------------")
message(STATUS "")

# -- LibRaw library Compilation --------------------------------------------------------------------------------

# Flag to include demosaic pack GPL2
add_definitions(-DLIBRAW_DEMOSAIC_PACK_GPL2)

# Flag to include demosaic pack GPL3
add_definitions(-DLIBRAW_DEMOSAIC_PACK_GPL3)

# Flag to compile with DNG SDK
add_definitions(-DUSE_DNGSDK)

# Flag to debug LibRaw
add_definitions(-DDCRAW_VERBOSE)

# Flag used by default into LibRaw to not use dedicated external thread
#add_definitions(-DLIBRAW_NOTHREADS)

# Flag to export library symbols
if(WIN32)
    if(MSVC)
        add_definitions(-DLIBRAW_BUILDLIB)
    else()
        add_definitions(-DLIBRAW_NODLL)
    endif()
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/
                    ${CMAKE_CURRENT_SOURCE_DIR}/demosaic-pack-GPL2
                    ${CMAKE_CURRENT_SOURCE_DIR}/demosaic-pack-GPL3
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../dngwriter/extra/dng_sdk
                   )

set(libraw_LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/internal/dcraw_common.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/internal/dcraw_fileio.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/internal/demosaic_packs.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/libraw_cxx.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/libraw_c_api.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/libraw_datastream.cpp
   )

# Disable compilation warnings from LibRaw. Just to be clear on the console.
# Adjust flag for static lib and 64 bits computers using -fPIC for GCC compiler (B.K.O: #269903)
# Use O4 GCC compilation option to prevent artifacts with OpenMP (B.K.O #305382)
foreach(_curentfile ${libraw_LIB_SRCS})
    if(WIN32 AND MSVC)
        set_source_files_properties(${_curentfile} PROPERTIES COMPILE_FLAGS "-w")
    else()
        set_source_files_properties(${_curentfile} PROPERTIES COMPILE_FLAGS "-w -fPIC -O4")
    endif()
endforeach()

add_library(libraw STATIC ${libraw_LIB_SRCS})

target_link_libraries(libraw
                      libdng
                      ${MATH_LIBRARY}
                      ${LCMS2_LIBRARIES}
                      ${OPENMP_LDFLAGS}
                     )
if(JPEG_FOUND)
    target_link_libraries(libraw ${JPEG_LIBRARY})
endif()

if(Jasper_FOUND)
    target_link_libraries(libraw ${JASPER_LIBRARIES})
endif()

# Registration of Libraw configuration to a dedicated header

MACRO_BOOL_TO_01(JPEG8_FOUND                      LIBRAW_USE_DNGLOSSYCODEC)
MACRO_BOOL_TO_01(OPENMP_FOUND                     LIBRAW_USE_OPENMP)
MACRO_BOOL_TO_01(Jasper_FOUND                     LIBRAW_USE_REDCINECODEC)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libraw_config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/libraw/libraw_config.h)
