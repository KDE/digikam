
SET(digikamimageplugin_inpainting_PART_SRCS imageplugin_inpainting.cpp
                                            inpaintingtool.cpp 
   )

KDE4_ADD_PLUGIN(digikamimageplugin_inpainting ${digikamimageplugin_inpainting_PART_SRCS})

TARGET_LINK_LIBRARIES(
    digikamimageplugin_inpainting
    digikamcore
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    )

INSTALL(TARGETS digikamimageplugin_inpainting DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL(FILES digikamimageplugin_inpainting_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_inpainting.desktop DESTINATION ${SERVICES_INSTALL_DIR})
