INCLUDE_DIRECTORIES(../utilities/imageeditor/editor
                    ../utilities/imageeditor/canvas
                     ../libs/widgets/common
                     ../libs/widgets/imageplugins
                     ../libs/dimg
                     ../libs/dimg/filters
                     ../libs/dmetadata
                    ../imageplugins/freerotation
)
SET( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

SET(manualrenameinputtest_SRCS
    manualrenameinputtest.cpp
)
kde4_add_unit_test(manualrenameinputtest ${manualrenameinputtest_SRCS})
TARGET_LINK_LIBRARIES(manualrenameinputtest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      digikamcore
                      )

SET(freerotationtest_SRCS
    ../imageplugins/freerotation/freerotationtool.cpp
    ../imageplugins/freerotation/freerotation.cpp
    ../imageplugins/freerotation/imageplugin_freerotation.cpp
    ../utilities/imageeditor/editor/editortool.cpp
    freerotationtest.cpp
)
kde4_add_unit_test(freerotationtest ${freerotationtest_SRCS})
TARGET_LINK_LIBRARIES(freerotationtest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      digikamcore
                      )
