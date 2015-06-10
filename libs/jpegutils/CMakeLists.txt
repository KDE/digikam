#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (c) 2015 by Veaceslav Munteanu, <veaceslav dot munteanu90 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Define libjpeg_EXPORTS: kde-win emerged libjpeg uses this define to
# decide wether to make dllimport (by default) or dllexport. So, we need to export.
add_definitions(-Dlibjpeg_EXPORTS)

set(libjpegutils_SRCS
    jpegutils.cpp
    iccjpeg.c
    ${DIGIKAM_LIBJPEG_DIR}/transupp.c
)

if(MSVC)
    set(libjpegutils_SRCS
        ${libjpegutils_SRCS}
        jpegwin.cpp
    )
endif()

add_library(jpegutils_src OBJECT ${libjpegutils_SRCS})

