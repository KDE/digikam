#
# Copyright (c) 2010-2018 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (POLICY CMP0063)
    cmake_policy(SET CMP0063 NEW)
endif (POLICY CMP0063)

set(detectcompression_SRCS detectcompression.cpp)
add_executable(detectcompression ${detectcompression_SRCS})
ecm_mark_nongui_executable(detectcompression)
target_link_libraries(detectcompression digikamcore libdng ${OpenCV_LIBRARIES})
