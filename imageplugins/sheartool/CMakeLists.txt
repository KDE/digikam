
SET(digikamimageplugin_sheartool_PART_SRCS imageplugin_sheartool.cpp
                                           imageeffect_sheartool.cpp
                                           sheartool.cpp 
   )


KDE4_ADD_PLUGIN(digikamimageplugin_sheartool ${digikamimageplugin_sheartool_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_sheartool digikam)

INSTALL(TARGETS digikamimageplugin_sheartool DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_sheartool_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_sheartool.desktop DESTINATION ${SERVICES_INSTALL_DIR})
