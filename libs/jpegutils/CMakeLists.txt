#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

# Define libjpeg_EXPORTS: kde-win emerged libjpeg uses this define to
# decide wether to make dllimport (by default) or dllexport. So, we need to export.
add_definitions(-Dlibjpeg_EXPORTS)

set(libjpegutils_SRCS
    jpegutils.cpp
    iccjpeg.c
    ${DIGIKAM_LIBJPEG_DIR}/transupp.c
)

# no warnings from JPEG extra implementations
if (NOT MSVC)
    set_source_files_properties(${DIGIKAM_LIBJPEG_DIR}/transupp.c PROPERTIES COMPILE_FLAGS "-w")
endif()

if(WIN32)
    set(libjpegutils_SRCS
        ${libjpegutils_SRCS}
        jpegwin.cpp
    )
endif()

include_directories(
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(jpegutils_src OBJECT ${libjpegutils_SRCS})
