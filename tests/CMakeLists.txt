include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../libs/widgets/common )
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set(manualrenameinputtest_SRCS manualrenameinputtest.cpp )
kde4_add_unit_test(manualrenameinputtest ${manualrenameinputtest_SRCS})
target_link_libraries(manualrenameinputtest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      digikamcore
                      )