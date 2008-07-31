
SET(digikamimageplugin_filmgrain_PART_SRCS imageplugin_filmgrain.cpp
                                           imageeffect_filmgrain.cpp
                                           filmgrain.cpp
   )


KDE4_ADD_PLUGIN(digikamimageplugin_filmgrain ${digikamimageplugin_filmgrain_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_filmgrain digikam)

INSTALL(TARGETS digikamimageplugin_filmgrain DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_filmgrain_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_filmgrain.desktop DESTINATION ${SERVICES_INSTALL_DIR})
