
SET(digikamimageplugin_colorfx_PART_SRCS imageplugin_colorfx.cpp 
                                         colorfxtool.cpp 
   )


KDE4_ADD_PLUGIN(digikamimageplugin_colorfx ${digikamimageplugin_colorfx_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_colorfx
    digikam
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDCRAW_LIBRARIES}    
    )

INSTALL(TARGETS digikamimageplugin_colorfx DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_colorfx_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_colorfx.desktop DESTINATION ${SERVICES_INSTALL_DIR})
