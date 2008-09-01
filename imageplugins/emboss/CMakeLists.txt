
SET(digikamimageplugin_emboss_PART_SRCS imageplugin_emboss.cpp
                                        embosstool.cpp 
                                        emboss.cpp 
   )


KDE4_ADD_PLUGIN(digikamimageplugin_emboss ${digikamimageplugin_emboss_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_emboss
    digikam
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDCRAW_LIBRARIES}
    )

INSTALL(TARGETS digikamimageplugin_emboss DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_emboss_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
install(FILES digikamimageplugin_emboss.desktop DESTINATION ${SERVICES_INSTALL_DIR})
