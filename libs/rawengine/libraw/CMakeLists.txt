#
# Copyright (c) 2010-2016, Gilles Caulier, <caulier dot gilles at gmail dot com>
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

# Under OSX, remove unwanted Clang compiler options

if(APPLE)
 
	while(CMAKE_CXX_FLAGS MATCHES "-Werror=return-type")
    	string(REPLACE "-Werror=return-type" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	endwhile()

	while(CMAKE_CXX_FLAGS MATCHES "-std=c++0x")
    	string(REPLACE "-std=c++0x" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	endwhile()

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

if(JASPER_FOUND)
    add_definitions(-DUSE_JASPER)
    include_directories(${JASPER_INCLUDE_DIR})
    message(STATUS "RawEngine will be compiled with RedCine codec")
else()
    message(STATUS "RawEngine will not be compiled with RedCine codec")
endif()

# For RawSpeed Codec Support

message(STATUS "Looking for LibXml2")
find_package(LibXml2)

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

if(JPEG8_FOUND AND LIBXML2_FOUND AND PTHREADS_FOUND)

    message(STATUS "RawEngine will be compiled with RawSpeed codec")
    set(RAWSPEED_SUPPORT_CAN_BE_COMPILED true)

else()

    # Show a detail of missing dependency for RawSpeed Codec support

    if(NOT JPEG8_FOUND)
        message(STATUS "RawEngine will not be compiled with RawSpeed codec. Libjpeg >= version 8 not found.")
    endif()

    if(NOT LIBXML2_FOUND)
        message(STATUS "RawEngine will not be compiled with RawSpeed codec. Libxml2 not found.")
    endif()

    if(NOT PTHREADS_FOUND)
        message(STATUS "RawEngine will not be compiled with RawSpeed codec. Pthreads not found.")
    endif()

    set(RAWSPEED_SUPPORT_CAN_BE_COMPILED false)

endif()

message(STATUS "--------------------------------------------------")
message(STATUS "")

# -- RAWSpeed Codec Compilation --------------------------------------------------------------------------------

if(RAWSPEED_SUPPORT_CAN_BE_COMPILED)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed
                        ${LIBXML2_INCLUDE_DIR})

    # Flag to include RawSpeed codec with Libraw
    add_definitions(-DUSE_RAWSPEED ${LIBXML2_DEFINITIONS})

    set(librawspeed_LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/ArwDecoder.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/BitPumpJPEG.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/BitPumpMSB.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/BitPumpMSB32.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/BitPumpPlain.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/BlackArea.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/ByteStream.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/ByteStreamSwap.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/Camera.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/CameraMetaData.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/CameraMetadataException.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/CameraSensorInfo.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/ColorFilterArray.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/Common.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/Cr2Decoder.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/DngDecoder.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/DngDecoderSlices.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/DngOpcodes.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/FileIOException.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/FileMap.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/IOException.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/LJpegDecompressor.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/LJpegPlain.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/NefDecoder.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/NikonDecompressor.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/OrfDecoder.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/PefDecoder.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/PentaxDecompressor.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/RawDecoder.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/RawDecoderException.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/RawImage.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/RawImageDataFloat.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/RawImageDataU16.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/RawParser.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/Rw2Decoder.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/SrwDecoder.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/TiffEntry.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/TiffEntryBE.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/TiffIFD.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/TiffIFDBE.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/TiffParser.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/TiffParserException.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/TiffParserHeaderless.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/RawSpeed/TiffParserOlympus.cpp
    )

endif()

# -- LibRaw library Compilation --------------------------------------------------------------------------------

# Flag to include demosaic pack GPL2
add_definitions(-DLIBRAW_DEMOSAIC_PACK_GPL2)

# Flag to include demosaic pack GPL3
add_definitions(-DLIBRAW_DEMOSAIC_PACK_GPL3)

# Flag to debug LibRaw
add_definitions(-DDCRAW_VERBOSE)

# Flag used by default into LibRaw to not use dedicated external thread
add_definitions(-DLIBRAW_NOTHREADS)

# Flag to export library symbols
if(WIN32)
    add_definitions(-DLIBRAW_BUILDLIB)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/
                    ${CMAKE_CURRENT_SOURCE_DIR}/demosaic-pack-GPL2
                    ${CMAKE_CURRENT_SOURCE_DIR}/demosaic-pack-GPL3
                   )

set(libraw_LIB_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/internal/dcraw_common.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/internal/dcraw_fileio.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/internal/demosaic_packs.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/libraw_cxx.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/libraw_c_api.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/libraw_datastream.cpp
   )

if(RAWSPEED_SUPPORT_CAN_BE_COMPILED)
    set(libraw_LIB_SRCS ${libraw_LIB_SRCS} ${librawspeed_LIB_SRCS})
endif()

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
                      ${MATH_LIBRARY}
                      ${LCMS2_LIBRARIES}
                      ${OPENMP_LDFLAGS}
                     )
if(JPEG_FOUND)
    target_link_libraries(libraw ${JPEG_LIBRARY})
endif()

if(JASPER_FOUND)
    target_link_libraries(libraw ${JASPER_LIBRARIES})
endif()

if(RAWSPEED_SUPPORT_CAN_BE_COMPILED)
    target_link_libraries(libraw ${LIBXML2_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
endif()

# Registration of Libraw configuration to a dedicated header

MACRO_BOOL_TO_01(JPEG8_FOUND                      LIBRAW_USE_DNGLOSSYCODEC)
MACRO_BOOL_TO_01(OPENMP_FOUND                     LIBRAW_USE_OPENMP)
MACRO_BOOL_TO_01(JASPER_FOUND                     LIBRAW_USE_REDCINECODEC)
MACRO_BOOL_TO_01(RAWSPEED_SUPPORT_CAN_BE_COMPILED LIBRAW_USE_RAWSPEED)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/libraw_config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/libraw/libraw_config.h)

# -- LibRaw CLI Samples Compilation --------------------------------------------------------------------------------

# A small macro so that this is a bit cleaner

macro(LIBRAW_BUILD_SAMPLES)

    set(_filename ${ARGV0})
    string(REPLACE "." ";" _temp ${_filename})
    list(GET _temp 0 _target) 

    set(${_target}_SRCS samples/${_filename})
    set_source_files_properties(${${_target}_SRCS} PROPERTIES COMPILE_FLAGS -w)

    add_executable(${_target} ${${_target}_SRCS})

    target_link_libraries(${_target} libraw ${OPENMP_LDFLAGS} ${CMAKE_THREAD_LIBS_INIT})

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
   if(WIN32)
        LIBRAW_BUILD_SAMPLES(half_mt_win32.c)
   else()
        LIBRAW_BUILD_SAMPLES(dcraw_half.c)
        LIBRAW_BUILD_SAMPLES(half_mt.c)
   endif()
endif()
