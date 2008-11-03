
SET(digikamimageplugin_adjustlevels_PART_SRCS imageplugin_adjustlevels.cpp
                                              adjustlevelstool.cpp 
   )

KDE4_ADD_PLUGIN(digikamimageplugin_adjustlevels ${digikamimageplugin_adjustlevels_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_adjustlevels
    digikamcore
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDCRAW_LIBRARIES}
    )

INSTALL(TARGETS digikamimageplugin_adjustlevels DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(FILES digikamimageplugin_adjustlevels_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_adjustlevels.desktop DESTINATION ${SERVICES_INSTALL_DIR})
