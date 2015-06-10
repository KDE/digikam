#
# Copyright (c) 2010-2015 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(detectcompression_SRCS detectcompression.cpp)
add_executable(detectcompression ${detectcompression_SRCS})
ecm_mark_nongui_executable(detectcompression)
target_link_libraries(detectcompression digikamcore ${OpenCV_LIBRARIES})

if(${KF5KGeoMap_FOUND})
    target_link_libraries(detectcompression ${KF5KGeoMap_LIBRARIES})
endif()
