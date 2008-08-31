
SET(digikamimageplugin_inserttext_PART_SRCS inserttextwidget.cpp
                                            imageplugin_inserttext.cpp
                                            inserttexttool.cpp
   )


KDE4_ADD_PLUGIN(digikamimageplugin_inserttext ${digikamimageplugin_inserttext_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_inserttext
    digikam
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    )

INSTALL(TARGETS digikamimageplugin_inserttext DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_inserttext_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_inserttext.desktop DESTINATION ${SERVICES_INSTALL_DIR})
