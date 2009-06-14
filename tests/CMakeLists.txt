SET( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/editor
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/canvas
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/widgets/common
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/widgets/imageplugins
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dmetadata
                    ${CMAKE_CURRENT_SOURCE_DIR}/../imageplugins/freerotation
)

#------------------------------------------------------------------------

SET(manualrenameinputtest_SRCS
    manualrenameinputtest.cpp
)
KDE4_ADD_UNIT_TEST(manualrenameinputtest ${manualrenameinputtest_SRCS})
TARGET_LINK_LIBRARIES(manualrenameinputtest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      digikamcore
                      )

#------------------------------------------------------------------------

SET(freerotationtest_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/../imageplugins/freerotation/freerotation.cpp
    freerotationtest.cpp
)
KDE4_ADD_UNIT_TEST(freerotationtest ${freerotationtest_SRCS})
TARGET_LINK_LIBRARIES(freerotationtest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      digikamcore
                      )

#------------------------------------------------------------------------

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../libs/threadimageio
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/3rdparty/libpgf
                   )

# Disable libpgf warnings.
SET_SOURCE_FILES_PROPERTIES(${libpgf_SRCS} PROPERTIES COMPILE_FLAGS "-w")

SET(qtpgftest_SRCS qtpgftest.cpp ${libpgf_SRCS} ${pgfutils_SRCS})
KDE4_ADD_EXECUTABLE(qtpgftest NOGUI ${qtpgftest_SRCS})
TARGET_LINK_LIBRARIES(qtpgftest ${KDE4_KIO_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

SET(pgfscaled_SRCS pgfscaled.cpp ${libpgf_SRCS} ${pgfutils_SRCS})
KDE4_ADD_EXECUTABLE(pgfscaled NOGUI ${pgfscaled_SRCS})
TARGET_LINK_LIBRARIES(pgfscaled ${KDE4_KIO_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

