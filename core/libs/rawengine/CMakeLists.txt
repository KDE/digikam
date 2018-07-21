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
else()
    add_definitions(-DLIBRAW_BUILDLIB)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libraw
                    ${CMAKE_CURRENT_SOURCE_DIR}/../dngwriter/extra/dng_sdk
                   )

set(libraw_LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/libraw/internal/dcraw_common.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/libraw/internal/dcraw_fileio.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/libraw/internal/demosaic_packs.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/libraw/src/libraw_cxx.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/libraw/src/libraw_c_api.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/libraw/src/libraw_datastream.cpp
   )

# Disable compilation warnings from LibRaw. Just to be clear on the console.
# Adjust flag for static lib and 64 bits computers using -fPIC for GCC compiler (Bug: #269903)
# Use O4 GCC compilation option to prevent artifacts with OpenMP (Bug #305382)
foreach(_curentfile ${libraw_LIB_SRCS})
    if(WIN32 AND MSVC)
        set_source_files_properties(${_curentfile} PROPERTIES COMPILE_FLAGS "-w")
    else()
        set_source_files_properties(${_curentfile} PROPERTIES COMPILE_FLAGS "-w -fPIC -O4")
    endif()
endforeach()

#################################

set(librawengine_SRCS
    drawdecoder.cpp
    drawdecoder_p.cpp
    drawdecodersettings.cpp
    drawdecoderwidget.cpp
    rawinfo.cpp
    ${libraw_LIB_SRCS}
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/libraw/
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(rawengine_srcs OBJECT ${librawengine_SRCS})
