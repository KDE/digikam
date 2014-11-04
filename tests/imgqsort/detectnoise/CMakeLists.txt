set(detectnoise_SRCS detectnoise.cpp)
KDE4_ADD_EXECUTABLE(detectnoise NOGUI ${detectnoise_SRCS})
target_link_libraries(detectnoise ${OpenCV_LIBRARIES})
