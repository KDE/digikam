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

                      # To link under Solaris (see B.K.O #274484)
                      ${MATH_LIBRARY}
                     )

INSTALL(TARGETS digikamdatabaseserver DESTINATION ${LIBEXEC_INSTALL_DIR})
