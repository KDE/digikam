SET(digikamimageplugin_decorate_PART_SRCS imageplugin_decorate.cpp
                                          inserttextwidget.cpp
                                          inserttexttool.cpp
                                          bordertool.cpp
                                          texturetool.cpp
   )

KDE4_ADD_PLUGIN(digikamimageplugin_decorate ${digikamimageplugin_decorate_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_decorate
    digikamcore
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDCRAW_LIBRARIES}
    )

INSTALL(TARGETS digikamimageplugin_decorate DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(FILES digikamimageplugin_decorate_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_decorate.desktop DESTINATION ${SERVICES_INSTALL_DIR})
