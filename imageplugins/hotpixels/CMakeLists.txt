
include_directories( ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas ${CMAKE_SOURCE_DIR}/digikam/libs/histogram ${CMAKE_SOURCE_DIR}/digikam/libs/levels ${CMAKE_SOURCE_DIR}/digikam/libs/curves ${CMAKE_SOURCE_DIR}/digikam/libs/whitebalance ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters ${CMAKE_SOURCE_DIR}/digikam/digikam   )


########### next target ###############

set(digikamimageplugin_hotpixels_PART_SRCS blackframeparser.cpp weights.cpp hotpixelfixer.cpp imageplugin_hotpixels.cpp blackframelistview.cpp imageeffect_hotpixels.cpp )

kde4_automoc(${digikamimageplugin_hotpixels_PART_SRCS})

kde4_add_plugin(digikamimageplugin_hotpixels ${digikamimageplugin_hotpixels_PART_SRCS})

target_link_libraries(digikamimageplugin_hotpixels  digikam )

install(TARGETS digikamimageplugin_hotpixels DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  digikamimageplugin_hotpixels_ui.rc DESTINATION ${DATA_INSTALL_DIR}/digikam )
install( FILES  digikamimageplugin_hotpixels.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


