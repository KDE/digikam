# Digikam Database Server #######################################################
SET(digikamdatabaseserver_SRCS ${lib..._SRCS}
				 databaseserver.cpp
				 pollthread.cpp
				 	
                 main.cpp
    )
    
QT4_ADD_DBUS_ADAPTOR(digikamdatabaseserver_SRCS org.kde.digikam.DatabaseServer.xml
                     databaseserver.h DatabaseServer)
                         
KDE4_ADD_EXECUTABLE(digikamdatabaseserver ${digikamdatabaseserver_SRCS})

TARGET_LINK_LIBRARIES(digikamdatabaseserver
                      ${KDE4_SOLID_LIBS}
                      ${KIPI_LIBRARIES}
                      ${KEXIV2_LIBRARIES}
                      ${KDCRAW_LIBRARIES}
                      ${QT_QTSQL_LIBRARY}
                      ${QT_QT3SUPPORT_LIBRARY}
                      ${STRIGI_LIBRARIES}
                      ${LCMS_LIBRARIES}
                      ${GPHOTO2_LIBRARIES}
                      digikamcore
                      digikamdatabase
                     )

IF(KDEPIMLIBS_FOUND)
  TARGET_LINK_LIBRARIES(digikamdatabaseserver ${KDE4_KABC_LIBS})
ENDIF(KDEPIMLIBS_FOUND)

IF(MARBLEWIDGET_FOUND)
  TARGET_LINK_LIBRARIES(digikamdatabaseserver ${LIBMARBLEWIDGET_LIBRARY})
ENDIF(MARBLEWIDGET_FOUND)

INSTALL(TARGETS digikamdatabaseserver ${INSTALL_TARGETS_DEFAULT_ARGS})
