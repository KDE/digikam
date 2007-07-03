
add_subdirectory( sharpnesseditor )
add_subdirectory( hsl )
add_subdirectory( ratiocrop )
add_subdirectory( sharpnesseditor )
add_subdirectory( hsl )
add_subdirectory( ratiocrop )

include_directories( ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas ${CMAKE_SOURCE_DIR}/digikam/libs/histogram ${CMAKE_SOURCE_DIR}/digikam/libs/levels ${CMAKE_SOURCE_DIR}/digikam/libs/curves ${CMAKE_SOURCE_DIR}/digikam/libs/whitebalance ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters ${CMAKE_SOURCE_DIR}/digikam/digikam ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/sharpnesseditor ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/hsl ${CMAKE_SOURCE_DIR}/digikam/imageplugins/coreplugin/ratiocrop   )


########### next target ###############

set(digikamimageplugin_core_PART_SRCS imageplugin_core.cpp imageeffect_bwsepia.cpp imageeffect_autocorrection.cpp imageeffect_rgb.cpp imageeffect_redeye.cpp imageeffect_blur.cpp imageeffect_iccproof.cpp imageeffect_bcg.cpp )


kde4_automoc(${digikamimageplugin_core_PART_SRCS})

kde4_add_plugin(digikamimageplugin_core ${digikamimageplugin_core_PART_SRCS})

target_link_libraries(digikamimageplugin_core  digikam )

install(TARGETS digikamimageplugin_core DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  digikamimageplugin_core_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam )
install( FILES  digikamimageplugin_core.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


