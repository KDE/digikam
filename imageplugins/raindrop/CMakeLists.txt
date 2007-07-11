
SET(digikamimageplugin_raindrop_PART_SRCS imageplugin_raindrop.cpp
                                          imageeffect_raindrop.cpp 
                                          raindrop.cpp 
   )


KDE4_ADD_PLUGIN(digikamimageplugin_raindrop ${digikamimageplugin_raindrop_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_raindrop digikam)

INSTALL(TARGETS digikamimageplugin_raindrop DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_raindrop_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam )
INSTALL(FILES digikamimageplugin_raindrop.desktop DESTINATION ${SERVICES_INSTALL_DIR})
