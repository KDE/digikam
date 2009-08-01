
SET(digikamimageplugin_localcontrast_PART_SRCS imageplugin_localcontrast.cpp
                                               localcontrast.cpp
                                               localcontrasttool.cpp
                                               ToneMappingBase.cpp
                                               ToneMappingFloat.cpp
                                               ToneMappingInt.cpp
                                               ToneMappingParameters.cpp
    )

KDE4_ADD_PLUGIN(digikamimageplugin_localcontrast ${digikamimageplugin_localcontrast_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_localcontrast
    digikamcore
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDCRAW_LIBRARIES}
    )

INSTALL(TARGETS digikamimageplugin_localcontrast DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(FILES digikamimageplugin_localcontrast_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_localcontrast.desktop DESTINATION ${SERVICES_INSTALL_DIR})
