
include_directories( ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/editor ${CMAKE_SOURCE_DIR}/digikam/utilities/imageeditor/canvas ${CMAKE_SOURCE_DIR}/digikam/libs/histogram ${CMAKE_SOURCE_DIR}/digikam/libs/levels ${CMAKE_SOURCE_DIR}/digikam/libs/curves ${CMAKE_SOURCE_DIR}/digikam/libs/whitebalance ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/common ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/iccprofiles ${CMAKE_SOURCE_DIR}/digikam/libs/widgets/imageplugins ${CMAKE_SOURCE_DIR}/digikam/libs/dialogs ${CMAKE_SOURCE_DIR}/digikam/libs/dimg ${CMAKE_SOURCE_DIR}/digikam/libs/dmetadata ${CMAKE_SOURCE_DIR}/digikam/libs/dimg/filters ${CMAKE_SOURCE_DIR}/digikam/digikam   )


########### next target ###############

set(digikamimageplugin_oilpaint_PART_SRCS imageplugin_oilpaint.cpp imageeffect_oilpaint.cpp oilpaint.cpp )


kde4_automoc(${digikamimageplugin_oilpaint_PART_SRCS})

kde4_add_plugin(digikamimageplugin_oilpaint ${digikamimageplugin_oilpaint_PART_SRCS})

target_link_libraries(digikamimageplugin_oilpaint  digikam )

install(TARGETS digikamimageplugin_oilpaint DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  digikamimageplugin_oilpaint_ui.rc         DESTINATION ${DATA_INSTALL_DIR}/digikam )
install( FILES  digikamimageplugin_oilpaint.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


