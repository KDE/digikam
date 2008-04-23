
SET(digikamimageplugin_antivignetting_PART_SRCS imageplugin_antivignetting.cpp
                                                imageeffect_antivignetting.cpp
                                                antivignetting.cpp 
   )


KDE4_ADD_PLUGIN(digikamimageplugin_antivignetting ${digikamimageplugin_antivignetting_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_antivignetting digikam )

INSTALL(TARGETS digikamimageplugin_antivignetting DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_antivignetting_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_antivignetting.desktop DESTINATION ${SERVICES_INSTALL_DIR})
