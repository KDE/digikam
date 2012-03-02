
SET(digikamimageplugin_color_PART_SRCS autocorrectiontool.cpp
                                       bcgtool.cpp
                                       bwsepiatool.cpp
                                       profileconversiontool.cpp
                                       cbtool.cpp
                                       whitebalancetool.cpp
                                       hsltool.cpp
                                       channelmixertool.cpp
                                       adjustcurvestool.cpp
                                       adjustlevelstool.cpp
                                       imageplugin_color.cpp
                                       filmtool.cpp
   )


KDE4_ADD_PLUGIN(digikamimageplugin_color ${digikamimageplugin_color_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_color
    digikamcore
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDCRAW_LIBRARIES}
    ${KEXIV2_LIBRARIES}
    ${QT_QT3SUPPORT_LIBRARY}
    )

INSTALL(TARGETS digikamimageplugin_color DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(FILES digikamimageplugin_color_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_color.desktop DESTINATION ${SERVICES_INSTALL_DIR})
