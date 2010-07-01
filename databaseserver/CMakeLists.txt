SET_DEBUG_AREA_CODE(${AREA_CODE_DATABASESERVER})

SET(digikamdatabaseserver_SRCS ${lib..._SRCS}
                               databaseserver.cpp
                               pollthread.cpp
                               main.cpp
   )

INCLUDE_DIRECTORIES(
                     # database libs
                     ${CMAKE_CURRENT_SOURCE_DIR}/../libs/database
                     ${CMAKE_CURRENT_SOURCE_DIR}/../digikam

                     # config-digikam.h
                     ${CMAKE_CURRENT_BINARY_DIR}/../
                   )

QT4_ADD_DBUS_ADAPTOR(digikamdatabaseserver_SRCS org.kde.digikam.DatabaseServer.xml
                     databaseserver.h Digikam::DatabaseServer)

KDE4_ADD_EXECUTABLE(digikamdatabaseserver ${digikamdatabaseserver_SRCS})

TARGET_LINK_LIBRARIES(digikamdatabaseserver
                      digikamcore
                      digikamdatabase
                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTSQL_LIBRARY}
                      ${QT_QT3SUPPORT_LIBRARY}
                     )

INSTALL(TARGETS digikamdatabaseserver DESTINATION ${LIBEXEC_INSTALL_DIR})
