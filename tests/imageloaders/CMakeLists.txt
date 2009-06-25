SET( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/dimg
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/dimg/filters
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/dimg/loaders
)

#------------------------------------------------------------------------

SET(imageloaderstest_SRCS
    imageloadertest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/dimg/loaders/dimgloader.cpp
)
    
KDE4_ADD_UNIT_TEST(imageloaderstest ${imageloaderstest_SRCS})
TARGET_LINK_LIBRARIES(imageloaderstest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      digikamcore
                      )