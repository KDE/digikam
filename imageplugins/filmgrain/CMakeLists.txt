
SET(digikamimageplugin_filmgrain_PART_SRCS imageplugin_filmgrain.cpp
                                           filmgraintool.cpp
                                           filmgrain.cpp
   )

KDE4_ADD_PLUGIN(digikamimageplugin_filmgrain ${digikamimageplugin_filmgrain_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_filmgrain
    digikamcore
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    )

INSTALL(TARGETS digikamimageplugin_filmgrain DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(FILES digikamimageplugin_filmgrain_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_filmgrain.desktop DESTINATION ${SERVICES_INSTALL_DIR})
