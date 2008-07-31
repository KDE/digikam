
SET(digikamimageplugin_adjustlevels_PART_SRCS imageplugin_adjustlevels.cpp
                                              adjustlevels.cpp 
   )


KDE4_ADD_PLUGIN(digikamimageplugin_adjustlevels ${digikamimageplugin_adjustlevels_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_adjustlevels digikam)

INSTALL(TARGETS digikamimageplugin_adjustlevels DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_adjustlevels_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
install(FILES digikamimageplugin_adjustlevels.desktop DESTINATION ${SERVICES_INSTALL_DIR})
