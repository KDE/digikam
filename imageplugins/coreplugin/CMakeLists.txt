
ADD_SUBDIRECTORY( sharpnesseditor )
ADD_SUBDIRECTORY( hsl )
ADD_SUBDIRECTORY( ratiocrop )
ADD_SUBDIRECTORY( sharpnesseditor )
ADD_SUBDIRECTORY( hsl )
ADD_SUBDIRECTORY( ratiocrop )

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor
                    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/hsl
                    ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/ratiocrop
                   )

SET(digikamimageplugin_core_PART_SRCS imageplugin_core.cpp 
                                      imageeffect_bwsepia.cpp 
                                      imageeffect_autocorrection.cpp 
                                      imageeffect_rgb.cpp 
                                      imageeffect_redeye.cpp 
                                      imageeffect_blur.cpp 
                                      imageeffect_iccproof.cpp 
                                      imageeffect_bcg.cpp 
   )

KDE4_AUTOMOC(${digikamimageplugin_core_PART_SRCS})

KDE4_ADD_PLUGIN(digikamimageplugin_core ${digikamimageplugin_core_PART_SRCS})

TARGET_LINK_LIBRARIES(digikamimageplugin_core digikam)

INSTALL(TARGETS digikamimageplugin_core DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL(FILES digikamimageplugin_core_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam)
INSTALL(FILES digikamimageplugin_core.desktop DESTINATION ${SERVICES_INSTALL_DIR})
