set(detectcompression_SRCS detectcompression.cpp)
KDE4_ADD_EXECUTABLE(detectcompression NOGUI ${detectcompression_SRCS})
target_link_libraries(detectcompression ${OpenCV_LIBRARIES})
