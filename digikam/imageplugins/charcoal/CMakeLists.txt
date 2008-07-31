
SET(digikamimageplugin_charcoal_PART_SRCS  imageplugin_charcoal.cpp
                                           imageeffect_charcoal.cpp 
                                           charcoal.cpp
   )


KDE4_ADD_PLUGIN(digikamimageplugin_charcoal ${digikamimageplugin_charcoal_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_charcoal digikam)

INSTALL(TARGETS digikamimageplugin_charcoal DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_charcoal_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
install(FILES digikamimageplugin_charcoal.desktop DESTINATION ${SERVICES_INSTALL_DIR})
