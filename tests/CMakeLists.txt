SET( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/editor
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/imageeditor/canvas
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/cameragui
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/manualrename
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/manualrename/parsers
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/manualrename/parsers/common
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/manualrename/parsers/subparsers
                    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/manualrename/parsers/modifiers
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/database
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/widgets/common
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/widgets/imageplugins
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/widgets/metadata
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dimg/filters
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/dmetadata
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libs/models
                    ${CMAKE_CURRENT_SOURCE_DIR}/../imageplugins/freerotation
                    ${CMAKE_CURRENT_SOURCE_DIR}/../digikam
                    ${CMAKE_CURRENT_SOURCE_DIR}/..
                    ${CMAKE_CURRENT_BINARY_DIR}/..
                    ${GPHOTO2_INCLUDE_DIRS}
)

#------------------------------------------------------------------------

SET(manualrenameinputtest_SRCS
    ${libmanualrename_SRCS}
    manualrenametest.cpp
)
KDE4_ADD_UNIT_TEST(manualrenameinputtest ${manualrenameinputtest_SRCS})
TARGET_LINK_LIBRARIES(manualrenameinputtest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      digikamcore
                      )

#------------------------------------------------------------------------

SET(cameranamehelper_SRCS
    cameranamehelpertest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../digikam/cameranamehelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/cameragui/gpcamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../utilities/cameragui/dkcamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../digikam/albumsettings.cpp
)
KDE4_ADD_UNIT_TEST(cameranamehelpertest ${cameranamehelper_SRCS})
TARGET_LINK_LIBRARIES(cameranamehelpertest
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}
                      ${GPHOTO2_LIBRARIES}
                      digikamcore
                      digikamdatabase
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

