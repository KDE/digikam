
SET(digikamimageplugin_perspective_PART_SRCS imageplugin_perspective.cpp
                                             imageeffect_perspective.cpp
                                             perspectivewidget.cpp 
                                             triangle.cpp 
                                             matrix.cpp 
   )


KDE4_ADD_PLUGIN(digikamimageplugin_perspective ${digikamimageplugin_perspective_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_perspective digikam)

INSTALL(TARGETS digikamimageplugin_perspective DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_perspective_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_perspective.desktop DESTINATION ${SERVICES_INSTALL_DIR})

