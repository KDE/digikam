
SET(digikamimageplugin_whitebalance_PART_SRCS imageplugin_whitebalance.cpp 
                                              whitebalancetool.cpp
   )


KDE4_ADD_PLUGIN(digikamimageplugin_whitebalance ${digikamimageplugin_whitebalance_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_whitebalance
    digikam
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KDE3SUPPORT_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDCRAW_LIBRARIES}    
    )

INSTALL(TARGETS digikamimageplugin_whitebalance DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_whitebalance_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_whitebalance.desktop DESTINATION ${SERVICES_INSTALL_DIR})
