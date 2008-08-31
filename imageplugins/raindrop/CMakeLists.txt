
SET(digikamimageplugin_raindrop_PART_SRCS imageplugin_raindrop.cpp
                                          raindroptool.cpp 
                                          raindrop.cpp 
   )


KDE4_ADD_PLUGIN(digikamimageplugin_raindrop ${digikamimageplugin_raindrop_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_raindrop
    digikam
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    )

INSTALL(TARGETS digikamimageplugin_raindrop DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_raindrop_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam )
INSTALL(FILES digikamimageplugin_raindrop.desktop DESTINATION ${SERVICES_INSTALL_DIR})
