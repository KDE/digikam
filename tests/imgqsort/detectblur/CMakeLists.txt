SET(detectblur_SRCS detectblur.cpp)
KDE4_ADD_EXECUTABLE(detectblur NOGUI ${detectblur_SRCS})
TARGET_LINK_LIBRARIES(detectblur ${OpenCV_LIBRARIES})
