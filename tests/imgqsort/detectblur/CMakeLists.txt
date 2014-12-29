#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(detectblur_SRCS detectblur.cpp)
add_executable(detectblur ${detectblur_SRCS})
ecm_mark_nongui_executable(detectblur)
target_link_libraries(detectblur ${OpenCV_LIBRARIES})
