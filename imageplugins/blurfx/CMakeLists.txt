
SET(digikamimageplugin_blurfx_PART_SRCS imageplugin_blurfx.cpp
                                        blurfxtool.cpp 
                                        blurfx.cpp 
   )

KDE4_ADD_PLUGIN(digikamimageplugin_blurfx ${digikamimageplugin_blurfx_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_blurfx
    digikamcore
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTGUI_LIBRARY}
    ${KDCRAW_LIBRARIES}
    )

INSTALL(TARGETS digikamimageplugin_blurfx DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(FILES digikamimageplugin_blurfx_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_blurfx.desktop DESTINATION ${SERVICES_INSTALL_DIR})
