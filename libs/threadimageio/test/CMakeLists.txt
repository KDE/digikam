INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/..
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/libpgf
                   )

# Disable libpgf warnings.
SET_SOURCE_FILES_PROPERTIES(${libpgf_SRCS} PROPERTIES COMPILE_FLAGS "-w")

#------------------------------------------------------------------------

SET(qtpgftest_SRCS qtpgftest.cpp ${libpgf_SRCS} ${pgfutils_SRCS})
KDE4_ADD_EXECUTABLE(qtpgftest NOGUI ${qtpgftest_SRCS})
TARGET_LINK_LIBRARIES(qtpgftest ${KDE4_KIO_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

#------------------------------------------------------------------------

SET(pgfscaled_SRCS pgfscaled.cpp ${libpgf_SRCS} ${pgfutils_SRCS})
KDE4_ADD_EXECUTABLE(pgfscaled NOGUI ${pgfscaled_SRCS})
TARGET_LINK_LIBRARIES(pgfscaled ${KDE4_KIO_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
