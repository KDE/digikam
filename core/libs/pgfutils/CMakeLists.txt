#
# Copyright (c) 2010-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015      by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT MSVC)

    REMOVE_COMPILER_WARNINGS("-Wcast-align")
    REMOVE_COMPILER_WARNINGS("-Wzero-as-null-pointer-constant")

    DISABLE_CLANG_COMPILER_WARNINGS("4.99.99" "-Wno-keyword-macro")

endif()

set(libpgfutils_SRCS
    pgfutils.cpp
    libpgf/Decoder.cpp
    libpgf/Encoder.cpp
    libpgf/PGFimage.cpp
    libpgf/PGFstream.cpp
    libpgf/Subband.cpp
    libpgf/WaveletTransform.cpp
)

include_directories(
    $<TARGET_PROPERTY:Qt5::Gui,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(pgfutils_src OBJECT ${libpgfutils_SRCS})
