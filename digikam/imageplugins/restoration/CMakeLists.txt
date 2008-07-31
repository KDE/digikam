
SET(digikamimageplugin_restoration_PART_SRCS imageplugin_restoration.cpp
                                             imageeffect_restoration.cpp 
   )


KDE4_ADD_PLUGIN(digikamimageplugin_restoration ${digikamimageplugin_restoration_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_restoration digikam )

INSTALL(TARGETS digikamimageplugin_restoration DESTINATION ${PLUGIN_INSTALL_DIR} )

INSTALL(FILES digikamimageplugin_restoration_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_restoration.desktop DESTINATION ${SERVICES_INSTALL_DIR})
