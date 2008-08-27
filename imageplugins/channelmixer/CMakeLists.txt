
SET(digikamimageplugin_channelmixer_PART_SRCS imageplugin_channelmixer.cpp
                                              channelmixer.cpp
   )


KDE4_ADD_PLUGIN(digikamimageplugin_channelmixer ${digikamimageplugin_channelmixer_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_channelmixer
    digikam
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDCRAW_LIBRARIES}
    )

INSTALL(TARGETS digikamimageplugin_channelmixer DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_channelmixer_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_channelmixer.desktop DESTINATION ${SERVICES_INSTALL_DIR})
