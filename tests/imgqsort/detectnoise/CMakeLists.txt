#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(detectnoise_SRCS detectnoise.cpp)
add_executable(detectnoise ${detectnoise_SRCS})
ecm_mark_nongui_executable(detectnoise)
target_link_libraries(detectnoise ${OpenCV_LIBRARIES})

if(${MARBLE_FOUND})
    target_link_libraries(detectnoise ${MARBLE_LIBRARIES})
endif()
