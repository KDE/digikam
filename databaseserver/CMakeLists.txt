SET_DEBUG_AREA_CODE(${AREA_CODE_DATABASESERVER})

SET(digikamdatabaseserver_SRCS ${lib..._SRCS}
                               databaseserver.cpp
                               pollthread.cpp
                               main.cpp
   )

QT4_ADD_DBUS_ADAPTOR(digikamdatabaseserver_SRCS org.kde.digikam.DatabaseServer.xml
                     databaseserver.h Digikam::DatabaseServer)

KDE4_ADD_EXECUTABLE(digikamdatabaseserver ${digikamdatabaseserver_SRCS})

TARGET_LINK_LIBRARIES(digikamdatabaseserver
                      digikamdatabase
                      digikamcore

                      ${KDE4_KDECORE_LIBS}
                      ${QT_QTSQL_LIBRARY}
                     )

IF(NOT WIN32)
    # To link under Solaris (see bug #274484)
    TARGET_LINK_LIBRARIES(digikamdatabaseserver ${MATH_LIBRARY})
ENDIF(NOT WIN32)

IF(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
    TARGET_LINK_LIBRARIES(digikamdatabaseserver ${KVM_LIBRARY})
ENDIF(CMAKE_SYSTEM_NAME STREQUAL FreeBSD)

INSTALL(TARGETS digikamdatabaseserver DESTINATION ${LIBEXEC_INSTALL_DIR})
