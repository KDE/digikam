#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(detectnoise_SRCS detectnoise.cpp)
add_executable(detectnoise ${detectnoise_SRCS})
ecm_mark_nongui_executable(detectnoise)
target_link_libraries(detectnoise
                      Qt5::Core
                      Qt5::Widgets
                      ${OpenCV_LIBRARIES})
