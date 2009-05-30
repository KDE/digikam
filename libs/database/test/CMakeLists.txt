INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/..
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libpgf
                   )

# Disable libpgf warnings.
FOREACH(_curentfile ${libpgf_SRCS})
    SET_SOURCE_FILES_PROPERTIES(${_curentfile} PROPERTIES COMPILE_FLAGS -w)
ENDFOREACH(_curentfile ${libpgf_SRCS})

SET(qtpgftest_SRCS qtpgftest.cpp
                   ../thumbnailpgf.cpp
                   ${libpgf_SRCS}
   )

KDE4_ADD_EXECUTABLE(qtpgftest NOGUI ${qtpgftest_SRCS})

TARGET_LINK_LIBRARIES(qtpgftest
                      ${KDE4_KIO_LIBS}
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                     )
