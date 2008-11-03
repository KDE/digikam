
SET(digikamimageplugin_distortionfx_PART_SRCS imageplugin_distortionfx.cpp
                                              distortionfxtool.cpp
                                              distortionfx.cpp
   )

KDE4_ADD_PLUGIN(digikamimageplugin_distortionfx ${digikamimageplugin_distortionfx_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_distortionfx
    digikamcore
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDCRAW_LIBRARIES}
    )

INSTALL(TARGETS digikamimageplugin_distortionfx DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(FILES digikamimageplugin_distortionfx_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_distortionfx.desktop DESTINATION ${SERVICES_INSTALL_DIR})
