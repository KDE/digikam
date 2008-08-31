
ADD_SUBDIRECTORY( patterns )

SET(digikamimageplugin_border_PART_SRCS imageplugin_border.cpp
                                        bordertool.cpp 
                                        border.cpp 
   )


KDE4_ADD_PLUGIN(digikamimageplugin_border ${digikamimageplugin_border_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_border
    digikam
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY}
    )

INSTALL(TARGETS digikamimageplugin_border DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_border_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
install(FILES digikamimageplugin_border.desktop DESTINATION ${SERVICES_INSTALL_DIR})
