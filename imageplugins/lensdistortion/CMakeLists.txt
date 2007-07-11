
SET(digikamimageplugin_lensdistortion_PART_SRCS imageplugin_lensdistortion.cpp
                                                imageeffect_lensdistortion.cpp
                                                lensdistortion.cpp
                                                pixelaccess.cpp
   )


KDE4_ADD_PLUGIN(digikamimageplugin_lensdistortion ${digikamimageplugin_lensdistortion_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_lensdistortion digikam)

INSTALL(TARGETS digikamimageplugin_lensdistortion DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_lensdistortion_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_lensdistortion.desktop DESTINATION ${SERVICES_INSTALL_DIR})
