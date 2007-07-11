
SET(digikamimageplugin_superimpose_PART_SRCS superimposewidget.cpp
                                             superimpose.cpp 
                                             dirselectwidget.cpp
                                             imageplugin_superimpose.cpp
                                             imageeffect_superimpose.cpp 
   )


KDE4_ADD_PLUGIN(digikamimageplugin_superimpose ${digikamimageplugin_superimpose_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_superimpose digikam ${KDE4_KFILE_LIBS})

INSTALL(TARGETS digikamimageplugin_superimpose DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_superimpose_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_superimpose.desktop DESTINATION ${SERVICES_INSTALL_DIR})
