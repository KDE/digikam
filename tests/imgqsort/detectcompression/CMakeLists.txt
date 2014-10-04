SET(detectcompression_SRCS detectcompression.cpp)
KDE4_ADD_EXECUTABLE(detectcompression NOGUI ${detectcompression_SRCS})
TARGET_LINK_LIBRARIES(detectcompression ${OpenCV_LIBRARIES})
