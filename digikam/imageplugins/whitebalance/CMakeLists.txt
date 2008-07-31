
SET(digikamimageplugin_whitebalance_PART_SRCS imageplugin_whitebalance.cpp 
                                              imageeffect_whitebalance.cpp
   )


KDE4_ADD_PLUGIN(digikamimageplugin_whitebalance ${digikamimageplugin_whitebalance_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_whitebalance digikam)

INSTALL(TARGETS digikamimageplugin_whitebalance DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_whitebalance_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_whitebalance.desktop DESTINATION ${SERVICES_INSTALL_DIR})
