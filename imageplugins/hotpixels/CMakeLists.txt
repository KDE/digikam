
SET(digikamimageplugin_hotpixels_PART_SRCS blackframeparser.cpp 
                                           weights.cpp 
                                           hotpixelfixer.cpp
                                           blackframelistview.cpp
                                           imageplugin_hotpixels.cpp
                                           imageeffect_hotpixels.cpp 
   )

KDE4_ADD_PLUGIN(digikamimageplugin_hotpixels ${digikamimageplugin_hotpixels_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_hotpixels digikam)

INSTALL(TARGETS digikamimageplugin_hotpixels DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_hotpixels_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_hotpixels.desktop DESTINATION ${SERVICES_INSTALL_DIR})
