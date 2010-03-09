
SET(digikamimageplugin_contentawareresizing_PART_SRCS
    imageplugin_contentawareresizing.cpp
    contentawareresizetool.cpp
   )

KDE4_ADD_PLUGIN(digikamimageplugin_contentawareresizing ${digikamimageplugin_contentawareresizing_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_contentawareresizing
                      digikamcore
                      ${KDCRAW_LIBRARIES}
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KDEUI_LIBS}
                      ${QT_QTCORE_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                     )

INSTALL(TARGETS digikamimageplugin_contentawareresizing DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(FILES digikamimageplugin_contentawareresizing_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_contentawareresizing.desktop DESTINATION ${SERVICES_INSTALL_DIR})
