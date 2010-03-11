
SET(digikamimageplugin_core_PART_SRCS autocorrectiontool.cpp
                                      bcgtool.cpp
                                      bwsepiatool.cpp
                                      profileconversiontool.cpp
                                      cbtool.cpp
                                      whitebalancetool.cpp
                                      hsltool.cpp
                                      channelmixertool.cpp
                                      adjustcurvestool.cpp
                                      adjustlevelstool.cpp
                                      imageplugin_core.cpp
   )


KDE4_ADD_PLUGIN(digikamimageplugin_core ${digikamimageplugin_core_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_core
    digikamcore
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDCRAW_LIBRARIES}
    )

INSTALL(TARGETS digikamimageplugin_core DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(FILES digikamimageplugin_core_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_core.desktop DESTINATION ${SERVICES_INSTALL_DIR})
