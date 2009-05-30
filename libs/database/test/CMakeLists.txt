INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/..
                    ${CMAKE_CURRENT_SOURCE_DIR}/../sqlite2
                    ${CMAKE_CURRENT_SOURCE_DIR}/../libpgf
                    ${CMAKE_CURRENT_SOURCE_DIR}/../haar
                    ${CMAKE_CURRENT_SOURCE_DIR}/../models
                   )

# Disable libsqlite2 C code warnings.
FOREACH(_curentfile ${libsqlite2_SRCS})
    SET_SOURCE_FILES_PROPERTIES(${_curentfile} PROPERTIES COMPILE_FLAGS -w)
ENDFOREACH(_curentfile ${libsqlite2_SRCS})

# Disable libpgf warnings.
FOREACH(_curentfile ${libpgf_SRCS})
    SET_SOURCE_FILES_PROPERTIES(${_curentfile} PROPERTIES COMPILE_FLAGS -w)
ENDFOREACH(_curentfile ${libpgf_SRCS})

SET(qtpgftest_SRCS qtpgftest.cpp
#                    ${libdatabase_SRCS}
#                    ${libhaar_SRCS}
#                    ${libdatabasemodels_SRCS}
#                    ${libsqlite2_SRCS}
                   ${libpgf_SRCS}
   )

KDE4_ADD_EXECUTABLE(qtpgftest NOGUI ${qtpgftest_SRCS})

TARGET_LINK_LIBRARIES(qtpgftest
                      ${KDE4_KIO_LIBS}
#                       ${KDE4_SOLID_LIBS}
#                       ${KEXIV2_LIBRARIES}
#                       ${KDCRAW_LIBRARIES}
#                       ${QT_QTSQL_LIBRARY}
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                     )
