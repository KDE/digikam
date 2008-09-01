
ADD_SUBDIRECTORY( patterns )

SET(digikamimageplugin_texture_PART_SRCS imageplugin_texture.cpp
                                         texturetool.cpp 
                                         texture.cpp 
   )


KDE4_ADD_PLUGIN(digikamimageplugin_texture ${digikamimageplugin_texture_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_texture
    digikam
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    )

INSTALL(TARGETS digikamimageplugin_texture DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_texture_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_texture.desktop DESTINATION ${SERVICES_INSTALL_DIR})
