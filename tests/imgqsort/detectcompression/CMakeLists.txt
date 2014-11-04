#
# Copyright (c) 2010-2014, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(detectcompression_SRCS detectcompression.cpp)
KDE4_ADD_EXECUTABLE(detectcompression NOGUI ${detectcompression_SRCS})
target_link_libraries(detectcompression ${OpenCV_LIBRARIES})
