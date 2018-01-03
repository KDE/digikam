#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(detectblur_SRCS detectblur.cpp)
add_executable(detectblur ${detectblur_SRCS})
ecm_mark_nongui_executable(detectblur)

target_link_libraries(detectblur
                      Qt5::Core
                      Qt5::Widgets
                      ${OpenCV_LIBRARIES})
