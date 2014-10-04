SET(detectnoise_SRCS detectnoise.cpp)
KDE4_ADD_EXECUTABLE(detectnoise NOGUI ${detectnoise_SRCS})
TARGET_LINK_LIBRARIES(detectnoise ${OpenCV_LIBRARIES})
