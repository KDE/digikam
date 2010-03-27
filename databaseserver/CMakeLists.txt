# Digikam Database Server #######################################################
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
                      ${KDE4_KDECORE_LIBS}
#                      ${KDE4_SOLID_LIBS}
#                      ${KIPI_LIBRARIES}
#                      ${KEXIV2_LIBRARIES}
#                      ${KDCRAW_LIBRARIES}
                      ${QT_QTSQL_LIBRARY}
                      ${QT_QT3SUPPORT_LIBRARY}
#                      ${STRIGI_LIBRARIES}
#                      ${LCMS_LIBRARIES}
#                      ${GPHOTO2_LIBRARIES}
                      digikamcore
                       digikamdatabase
                     )


INSTALL(TARGETS digikamdatabaseserver ${INSTALL_TARGETS_DEFAULT_ARGS})
