set(detectblur_SRCS detectblur.cpp)
KDE4_ADD_EXECUTABLE(detectblur NOGUI ${detectblur_SRCS})
target_link_libraries(detectblur ${OpenCV_LIBRARIES})
