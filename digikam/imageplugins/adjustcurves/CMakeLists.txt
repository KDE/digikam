
SET(digikamimageplugin_adjustcurves_PART_SRCS imageplugin_adjustcurves.cpp
                                              adjustcurves.cpp 
   )


KDE4_ADD_PLUGIN(digikamimageplugin_adjustcurves ${digikamimageplugin_adjustcurves_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_adjustcurves digikam)

INSTALL(TARGETS digikamimageplugin_adjustcurves DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_adjustcurves_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
install(FILES digikamimageplugin_adjustcurves.desktop DESTINATION ${SERVICES_INSTALL_DIR})
