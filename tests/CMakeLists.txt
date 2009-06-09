INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/../libs/widgets/common )
SET( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

SET(manualrenameinputtest_SRCS manualrenameinputtest.cpp )
kde4_add_unit_test(manualrenameinputtest ${manualrenameinputtest_SRCS})
TARGET_LINK_LIBRARIES(manualrenameinputtest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      digikamcore
                      )