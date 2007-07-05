
SET(digikamimageplugin_inserttext_PART_SRCS imageplugin_inserttext.cpp
                                            imageeffect_inserttext.cpp
                                            inserttextwidget.cpp
                                            fontchooserwidget.cpp 
   )

KDE4_AUTOMOC(${digikamimageplugin_inserttext_PART_SRCS})

KDE4_ADD_PLUGIN(digikamimageplugin_inserttext ${digikamimageplugin_inserttext_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_inserttext digikam)

INSTALL(TARGETS digikamimageplugin_inserttext DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES digikamimageplugin_inserttext_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
install(FILES digikamimageplugin_inserttext.desktop DESTINATION ${SERVICES_INSTALL_DIR})
