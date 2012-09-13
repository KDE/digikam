
SET(digikamimageplugin_fxfilters_PART_SRCS imageplugin_fxfilters.cpp
                                           colorfxtool.cpp
                                           charcoaltool.cpp
                                           embosstool.cpp
                                           oilpainttool.cpp
                                           blurfxtool.cpp
                                           distortionfxtool.cpp
                                           raindroptool.cpp
                                           filmgraintool.cpp
   )

KDE4_ADD_PLUGIN(digikamimageplugin_fxfilters ${digikamimageplugin_fxfilters_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_fxfilters
    digikamcore

    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${KDCRAW_LIBRARIES}

    ${QT_QT3SUPPORT_LIBRARY}
    )

INSTALL(TARGETS digikamimageplugin_fxfilters DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(FILES digikamimageplugin_fxfilters_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_fxfilters.desktop DESTINATION ${SERVICES_INSTALL_DIR})
