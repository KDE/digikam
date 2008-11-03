
SET(digikamimageplugin_freerotation_PART_SRCS freerotation.cpp 
                                              imageplugin_freerotation.cpp
                                              freerotationtool.cpp
   )

KDE4_ADD_PLUGIN(digikamimageplugin_freerotation ${digikamimageplugin_freerotation_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_freerotation
    digikamcore
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDCRAW_LIBRARIES}    
    )

INSTALL(TARGETS digikamimageplugin_freerotation DESTINATION ${PLUGIN_INSTALL_DIR} )
INSTALL( FILES digikamimageplugin_freerotation_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam )
INSTALL( FILES digikamimageplugin_freerotation.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


