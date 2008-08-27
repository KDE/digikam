
SET(digikamimageplugin_infrared_PART_SRCS imageplugin_infrared.cpp
                                          imageeffect_infrared.cpp 
                                          infrared.cpp 
   )


KDE4_ADD_PLUGIN(digikamimageplugin_infrared ${digikamimageplugin_infrared_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_infrared
    digikam
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    )

INSTALL(TARGETS digikamimageplugin_infrared DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_infrared_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_infrared.desktop DESTINATION ${SERVICES_INSTALL_DIR})
