
SET(digikamimageplugin_noisereduction_PART_SRCS imageplugin_noisereduction.cpp
                                                noisereductiontool.cpp
                                                noisereduction.cpp 
   )

KDE4_ADD_PLUGIN(digikamimageplugin_noisereduction ${digikamimageplugin_noisereduction_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_noisereduction
    digikamcore
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDCRAW_LIBRARIES}
    )

INSTALL(TARGETS digikamimageplugin_noisereduction DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(FILES digikamimageplugin_noisereduction_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_noisereduction.desktop DESTINATION ${SERVICES_INSTALL_DIR})
